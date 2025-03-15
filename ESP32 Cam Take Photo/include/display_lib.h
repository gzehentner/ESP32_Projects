#ifndef DisplayLib_h_
#define DisplayLib_h_

// Extern declaration of u8g2 instance
extern U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2;
    void u8g2_prepare();
    void u8g2_box_frame();
    void u8g2_r_frame_box();
    void u8g2_disc_circle();
    void u8g2_string_orientation();
    void u8g2_line();
    void u8g2_triangle();
    void u8g2_bitmap();

#endif