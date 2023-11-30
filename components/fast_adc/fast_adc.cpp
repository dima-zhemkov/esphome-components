#ifdef USE_ESP32

#include "fast_adc.h"
#include "esphome/core/log.h"

namespace esphome {
namespace fast_adc {

static const char *const TAG = "fast_adc";
static const uint32_t ADC_RESULT_BYTE = sizeof(adc_digi_output_data_t);

void FastADCComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Fast ADC...");

  this->pin_->setup();

  this->buffer_max_length_ = this->oversampling_ * ADC_RESULT_BYTE;

  uint8_t ch = static_cast<uint8_t>(this->channel_);

  adc_digi_init_config_t adc_dma_config = {
      .max_store_buf_size = 1024,
      .conv_num_each_intr = this->buffer_max_length_,
      .adc1_chan_mask = BIT(ch),
      .adc2_chan_mask = 0UL,
  };
  ESP_ERROR_CHECK(adc_digi_initialize(&adc_dma_config));

  adc_digi_pattern_config_t adc_pattern[]{{
      .atten = ADC_ATTEN_DB_11,
      .channel = ch,
      .unit = 0,
      .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH,
  }};

  adc_digi_configuration_t dig_cfg = {
      .conv_limit_en = 1,  // For ESP32, this should always be set to 1
      .conv_limit_num = 250,
      .pattern_num = sizeof(adc_pattern) / sizeof(adc_digi_pattern_config_t),
      .adc_pattern = adc_pattern,
      .sample_freq_hz = this->frequency_ * this->oversampling_,
      .conv_mode = ADC_CONV_SINGLE_UNIT_1,
      .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1,
  };

  ESP_ERROR_CHECK(adc_digi_controller_configure(&dig_cfg));

  auto cal_value = esp_adc_cal_characterize(ADC_UNIT_1, (adc_atten_t) ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12,
                                            1100,  // default vref
                                            &this->adc_cal_characteristics_);
  switch (cal_value) {
    case ESP_ADC_CAL_VAL_EFUSE_VREF:
      ESP_LOGV(TAG, "Using eFuse Vref for calibration");
      break;
    case ESP_ADC_CAL_VAL_EFUSE_TP:
      ESP_LOGV(TAG, "Using two-point eFuse Vref for calibration");
      break;
    case ESP_ADC_CAL_VAL_DEFAULT_VREF:
    default:
      break;
  }

  xTaskCreatePinnedToCore(&FastADCComponent::adc_task,
                          "fast_adc_task",            // name
                          4096,                       // stack size
                          static_cast<void *>(this),  // task pv params
                          23,                         // priority
                          nullptr,                    // handle
                          1                           // core
  );

  ESP_LOGCONFIG(TAG, "Fast ADC setup finished!");
}

void FastADCComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Fast ADC:");
  LOG_PIN("  Pin: ", this->pin_);

  ESP_LOGCONFIG(TAG, "  Frequency: %uHz", this->frequency_);
  ESP_LOGCONFIG(TAG, "  Oversampling: %u", this->oversampling_);
  ESP_LOGCONFIG(TAG, "  Calibration Data Size: %u", this->calibration_data_.size());
}

float FastADCComponent::get_setup_priority() const { return setup_priority::HARDWARE; }

void FastADCComponent::add_on_conversion_callback(std::function<void(float)> &&callback) {
  this->conversion_callback_.add(std::move(callback));
}

void FastADCComponent::adc_task(void *pv) {
  FastADCComponent *transfer_switch = static_cast<FastADCComponent *>(pv);

  const uint32_t max_length = transfer_switch->buffer_max_length_;
  const uint32_t oversampling = transfer_switch->oversampling_;
  const float mv_conversion_factor = 1.0f / 1000.0f;
  const float mv_conversion_oversampling_reciprocal = mv_conversion_factor / oversampling;

  float sum_mv = 0;
  uint16_t count = 0;

  ExternalRAMAllocator<uint8_t> allocator(ExternalRAMAllocator<uint8_t>::ALLOW_FAILURE);
  uint8_t *buffer = allocator.allocate(max_length);
  if (buffer == nullptr) {
    ESP_LOGE(TAG, "Could not allocate ADC buffer!");
    return;
  }

  ESP_ERROR_CHECK(adc_digi_start());

  while (true) {
    uint32_t length;
    auto state = adc_digi_read_bytes(buffer, max_length, &length, ADC_MAX_DELAY);

    if (state == ESP_ERR_INVALID_STATE) {
      ESP_ERROR_CHECK(adc_digi_stop());
      ESP_LOGW(TAG, "The conversion is too fast, whereas the task calling `adc_digi_read_bytes` is slow.");
      ESP_LOGW(TAG, "Either decrease the conversion speed, or increase the frequency you call `adc_digi_read_bytes`.");
      ESP_ERROR_CHECK(adc_digi_start());
    }

    for (int i = 0; i < length; i += ADC_RESULT_BYTE) {
      auto p = reinterpret_cast<adc_digi_output_data_t *>(&buffer[i]);
      sum_mv += transfer_switch->adc_cal_raw_to_mv(p->type1.data);
      count++;

      if (count == oversampling) {
        auto voltage = sum_mv * mv_conversion_oversampling_reciprocal;
        transfer_switch->conversion_callback_.call(voltage);
        sum_mv = 0;
        count = 0;
      }
    }
  }
}

float HOT FastADCComponent::adc_cal_raw_to_mv(uint16_t adc_reading) {
#ifdef USE_EXTERNAL_CALUBRATION
  return this->calibration_data_[adc_reading];
#else
  auto mv = esp_adc_cal_raw_to_voltage(adc_reading, &this->adc_cal_characteristics_);
  return static_cast<float>(mv);
#endif
}

}  // namespace fast_adc
}  // namespace esphome

#endif