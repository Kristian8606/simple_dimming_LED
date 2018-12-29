
#include <stdio.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>
#include <math.h> 

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <wifi_config.h>

#include "pwm.h"
#include "button.h"

#include "ota-api.h"

#define PWM_PIN 12
#define BUTTON_MODE_GPIO 0

float led_brightness = 100;     // brightness is scaled 0 to 100
bool led_on = false;            // on is boolean on or off

uint8_t press_button_count = 0;
int target_brightness = 0;
int current_brightness = 0;

void switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
void update_brightness(homekit_characteristic_t *ch, homekit_value_t value, void *context);

void on_led(bool on) {

    if(on){
   		 led_on = true;
    }else{
  	 	 led_on = false;
    }
}

void led_identify_task(void *_args) {
    for (int i = 0; i < 3; i++)
    {
        for (int i = 0; i < 3; i++)
        {
			pwm_set_duty(20000);
            vTaskDelay(150 / portTICK_PERIOD_MS);
            pwm_set_duty(0);
            vTaskDelay(150 / portTICK_PERIOD_MS);
        }

        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
    pwm_set_duty(0);

  

    vTaskDelete(NULL);
}

void led_identify(homekit_value_t _value) {
    xTaskCreate(led_identify_task, "LED identify", 128, NULL, 2, NULL);
}

void reset_configuration_task()
{
	 for (int i = 0; i < 5; i++)
        {
			pwm_set_duty(20000);
            vTaskDelay(150 / portTICK_PERIOD_MS);
            pwm_set_duty(0);
            vTaskDelay(150 / portTICK_PERIOD_MS);
        }
        pwm_set_duty(0);

    printf("Resetting Wifi Config\n");

    wifi_config_reset();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("Resetting HomeKit Config\n");

    homekit_server_reset();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("Restarting\n");

    sdk_system_restart();

    vTaskDelete(NULL);
}


homekit_characteristic_t brightness_set  = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 100, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(update_brightness));

homekit_characteristic_t switch_on = HOMEKIT_CHARACTERISTIC_(ON, false, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(switch_on_callback));

void switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    on_led(switch_on.value.bool_value);
  
}

void update_brightness(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    printf("update_brightness %d\n", brightness_set.value.int_value);
   
    
   
}

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, "LED Bulb");
homekit_characteristic_t ota_trigger  = API_OTA_TRIGGER;

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_lightbulb, .services = (homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
            &name,
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Kriso"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "737A2BAFF19E"),
            HOMEKIT_CHARACTERISTIC(MODEL, "LED Bulb"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.0.3"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, led_identify),
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "LED Bulb"),
            &switch_on,
		    &brightness_set,
		    &ota_trigger,
            NULL
        }),
        NULL
    }),
    NULL
};



homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-11-111"    //changed tobe valid
};


void pwm_bulb(void *pvParameters)
{
int x = 0;
    
     int pwm = 0;
 	
	
    while(1) {
         if (led_on) {
			target_brightness = (brightness_set.value.int_value * 100)*6.5;
        } else {
			target_brightness = 0;
      	
        }
      
      if(current_brightness < target_brightness){
      
      x = (target_brightness - current_brightness )/ 40;
    
 	  
 	  	pwm += x;
 	  if(pwm >= target_brightness){
      	pwm = target_brightness;
      	current_brightness = target_brightness;
      	}
      	
      //	printf("Up pwm %d Targrt %d Current %d\r\n", pwm,target_brightness,current_brightness);
      	pwm_set_duty(pwm);
      
      }else if(current_brightness > target_brightness){
      
 	 x = (current_brightness - target_brightness) / 40;
    
 	    pwm -= x;
      	if(pwm <= target_brightness){
      	pwm = target_brightness;
      	current_brightness = target_brightness;
     
      	}
      	
      	
    //  	printf("Down pwm %d Targrt %d Current %d\r\n", pwm,target_brightness,current_brightness);
      	pwm_set_duty(pwm);
      }
 
            
     vTaskDelay(1);
    }
}

void button_mode_callback(uint8_t gpio, button_event_t event)
{
    switch (event)
    {
    case button_event_single_press:
    press_button_count++;
    
    	switch (press_button_count)
    {
    	case 1:
    		brightness_set.value.int_value = 20;
    		if(switch_on.value.bool_value == false)
    		switch_on.value.bool_value = true;
    		
    	break;
    	case 2:
    		brightness_set.value.int_value = 60;
    		if(switch_on.value.bool_value == false)
    		switch_on.value.bool_value = true;
    		
    	break;
    	case 3:
    		brightness_set.value.int_value = 100;
    		if(switch_on.value.bool_value == false)
    		switch_on.value.bool_value = true;
    		
    	break;
    	default:
    		//led_brightness = 0;
    		press_button_count = 0;
    		switch_on.value.bool_value = false;
    		
    	break;
    }
    on_led(switch_on.value.bool_value);
    homekit_characteristic_notify(&brightness_set, brightness_set.value);
    homekit_characteristic_notify(&switch_on, switch_on.value);
    printf("press_button_count %d Target brightness %d\n",press_button_count , brightness_set.value.int_value);
  
        break;
    case button_event_long_press:
        xTaskCreate(reset_configuration_task, "Reset configuration", 256, NULL, 2, NULL);
        break;
   
    }
}

void pwm_task(){
	 uint8_t pins[1];
  
    pins[0] = PWM_PIN;
    pwm_init(1, pins, false);

    printf("pwm_set_freq(1000)     # 1 kHz\n");
    pwm_set_freq(1000);

    pwm_set_duty(0);

    printf("pwm_start()\n");
    pwm_start();
    gpio_enable(BUTTON_MODE_GPIO, GPIO_INPUT);
    
    if (button_create(BUTTON_MODE_GPIO, 0, 10000, button_mode_callback));
}

void on_wifi_ready() {
    homekit_server_init(&config);
}

void user_init(void) {
  // uart_set_baud(0, 115200);
    // This example shows how to use same firmware for multiple similar accessories
    // without name conflicts. It uses the last 3 bytes of accessory's MAC address as
    // accessory name suffix.
    pwm_task();
    
    uint8_t macaddr[6];
    sdk_wifi_get_macaddr(STATION_IF, macaddr);
    int name_len = snprintf(NULL, 0, "dimmable LED-%02X%02X%02X", macaddr[1], macaddr[2], macaddr[3]);
    char *name_value = malloc(name_len + 1);
    snprintf(name_value, name_len + 1, "dimmable LED-%02X%02X%02X", macaddr[1], macaddr[2], macaddr[3]);
    name.value = HOMEKIT_STRING(name_value);
    
    wifi_config_init("dimmable LED", NULL, on_wifi_ready);
    
	

    xTaskCreate(pwm_bulb, "pwm_bulb", 256, NULL, 2, NULL);
}
