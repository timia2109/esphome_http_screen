#include <functional>
#include "http_screen.h"
#include "esphome/core/log.h"
#include <string>

using namespace esphome;

namespace esphome
{
    namespace http_screen
    {
        static const char *const TAG = "http_screen";
        inline void Position::add_pixels(int quantity)
        {
            _x += quantity;

            while (_x >= _width)
            {
                _x -= _width;
                _y++;
            }

            if (_y > _height)
                _y = _height;
        }

        std::vector<char> *base64_decode(std::string const &encoded_string)
        {
            int in_len = encoded_string.size();
            int i = 0;
            int j = 0;
            int in = 0;
            char char_array_4[4], char_array_3[3];
            auto ret = new std::vector<char>();

            while (in_len-- && (encoded_string[in] != '=') && is_base64(encoded_string[in]))
            {
                char_array_4[i++] = encoded_string[in];
                in++;
                if (i == 4)
                {
                    for (i = 0; i < 4; i++)
                        char_array_4[i] = BASE64_CHARS.find(char_array_4[i]);

                    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                    for (i = 0; (i < 3); i++)
                        ret->push_back(char_array_3[i]);
                    i = 0;
                }
            }

            if (i)
            {
                for (j = i; j < 4; j++)
                    char_array_4[j] = 0;

                for (j = 0; j < 4; j++)
                    char_array_4[j] = BASE64_CHARS.find(char_array_4[j]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (j = 0; (j < i - 1); j++)
                    ret->push_back(char_array_3[j]);
            }

            return ret;
        }

        void HttpScreen::dump_config()
        {
            ESP_LOGCONFIG(TAG, "ready");
        }

        void HttpScreen::process(int32_t status_code, uint32_t duration_ms) {
            ESP_LOGI(TAG, "Process HTTP Response with SC: %d", status_code);

            if (status_code >= 300 || status_code < 200 || data_ != nullptr)
                return;

            // Get Response
            auto string_response = http_client_->get_string();
            auto string_object = new std::string(string_response);

            // Decode Base64
            ESP_LOGI("mqtt_screen", "Decode Base64");
            auto decodedData = base64_decode(*string_object);
            delete string_object;

            data_ = new std::vector<unsigned short>;

            for (size_t i = 0; i < decodedData->size(); i += 2)
            {
                if (i + 1 < decodedData->size())
                {
                    auto highByte = static_cast<unsigned short>((*decodedData)[i + 1]);
                    auto lowByte = static_cast<unsigned short>((*decodedData)[i]);
                    unsigned short value = (highByte << 8) | lowByte;
                    data_->push_back(value);
                }
            }
            delete decodedData;

            // Update Display
            ESP_LOGI(TAG, "Update Screen");
            this->display_->update();

            delete data_;
            data_ = nullptr;
        }

        void HttpScreen::setup()
        {
            display_->set_writer(
                std::bind(&HttpScreen::draw, this, std::placeholders::_1));

            trigger_ = new http_request::HttpRequestResponseTrigger();
            auto automation = new Automation<int32_t, uint32_t>(trigger_);
            auto lambdaaction = new LambdaAction<int32_t, uint32_t>(
                std::bind(&HttpScreen::process, this, std::placeholders::_1, std::placeholders::_2)
            );
            automation->add_actions({lambdaaction});
        }

        void HttpScreen::load_image(const std::string &url)
        {
            if (data_ != nullptr)
            {
                ESP_LOGI(TAG, "Already running an action. Skipping");
                return;
            }

            http_client_->set_method("GET");
            http_client_->set_url(url);

            ESP_LOGI(TAG, "Request image from HTTP");
            http_client_->send({ trigger_ });
            ESP_LOGI(TAG, "Finished HTTP");
        }

        void HttpScreen::draw(display::Display &display)
        {
            display.fill(display::COLOR_OFF);
            ESP_LOGI(TAG, "Begin drawing");

            Position pos(display.get_width(), display.get_height());

            for (std::vector<unsigned short>::iterator it = data_->begin(); it != data_->end(); ++it)
            {
                unsigned short plain_data = *it;
                unsigned short mode = plain_data >> 14;
                unsigned short payload = plain_data & ~(3 << 14);

                if (mode == 0)
                {
                    // Plain
                    for (int i = 13; i >= 0; i--)
                    {
                        unsigned short mask = 1 << i;
                        if (payload & mask)
                        {
                            display.draw_pixel_at(pos.get_x(), pos.get_y(), display::COLOR_ON);
                        }
                        pos++;
                    }
                }
                else if (mode == 1)
                {
                    // White (nothing to draw)
                    pos += payload;
                }
                else if (mode == 2)
                {
                    // Black
                    for (int i = payload; i > 0; i--)
                    {
                        display.draw_pixel_at(pos.get_x(), pos.get_y(), display::COLOR_ON);
                        pos++;
                    }
                }
            }

            ESP_LOGI(TAG, "Finished drawing");
        }

        float HttpScreen::get_setup_priority() const
        {
            return setup_priority::AFTER_CONNECTION;
        }
    }
}
