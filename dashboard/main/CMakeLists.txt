idf_component_register(SRCS "main.cpp"
                            "comp/slider/Slider.cpp"
                            "comp/wifi/WifiManager.cpp"
                            "comp/gfx/LVGLHelper.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES esp_wifi esp_adc)