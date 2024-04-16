#pragma once

#include "esphome.h"
using namespace esphome;

namespace esphome
{
    namespace http_screen
    {

        static const std::string BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

        static inline bool is_base64(char c) { return (isalnum(c) || (c == '+') || (c == '/')); }

        std::vector<char> base64_decode(std::string const &);

        class HttpScreen : public Component
        {
        public:
            void load_image(const std::string &url);
            void set_display(display::Display *display) { display_ = display; }
            void set_http_client(http_request::HttpRequestComponent *http_client) {
                http_client_ = http_client;
            }

            void process(int32_t status_code, uint32_t duration_ms);
            void draw(display::Display &display);

            void setup() override;
            void loop() override{};
            void dump_config() override;
            float get_setup_priority() const override;

        private:
            std::vector<unsigned short>* data_;

            http_request::HttpRequestResponseTrigger *trigger_;
            display::Display *display_;
            http_request::HttpRequestComponent *http_client_;
        };
    }
}