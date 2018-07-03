#ifdef USE_RPI_SENSE

#include <sense/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>



sense_bitmap_t fb;
sense_bitmap_t buffer;

void sense_init(){

  fb = sense_alloc_fb();
  if (!fb){
    fprintf(stderr,"Could not allocate framebuffer: %s\n",sense_strerror(sense_errno()));
    exit(1);
  }
  buffer = sense_alloc_bitmap();

}

void sense_set_pixel(int x, int y, int state){

  sense_color_t color;
  if(state == 1){
    color = sense_make_color_rgb(0xff, 0, 0);
  }else if (state == 2){
    color = sense_make_color_rgb(0, 0xff, 0);
  }else if (state ==  3){
    color = sense_make_color_rgb(0xff, 0xff, 0);
  }else{
    color = sense_make_color_rgb(0, 0, 0);
  }

  sense_bitmap_set_pixel(buffer, x, y, color);

}

void sense_update(){
  sense_bitmap_cpy(fb, buffer);
}

void sense_clear(){
  sense_bitmap_paint(fb, 0);
}

void sense_free(){
  sense_free_bitmap(fb);
  sense_free_bitmap(buffer);
}

#endif
