#control constants
IS_BLAZE_GATEWAY = 1

ifeq ($(IS_BLAZE_GATEWAY),1)
  CFLAGS += -DMM_BLAZE_GATEWAY
else
  CFLAGS += -DMM_BLAZE_NODE
endif

# Source files common to all targets
SRC_FILES += \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_serial.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c \
  $(SDK_ROOT)/components/boards/boards.c \
  $(SDK_ROOT)/components/libraries/button/app_button.c \
  $(SDK_ROOT)/components/libraries/util/app_error.c \
  $(SDK_ROOT)/components/libraries/util/app_error_weak.c \
  $(SDK_ROOT)/components/libraries/scheduler/app_scheduler.c \
  $(SDK_ROOT)/components/libraries/timer/app_timer.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/components/libraries/hardfault/nrf52/handler/hardfault_handler_gcc.c \
  $(SDK_ROOT)/components/libraries/hardfault/hardfault_implementation.c \
  $(SDK_ROOT)/components/libraries/util/nrf_assert.c \
  $(SDK_ROOT)/components/libraries/strerror/nrf_strerror.c \
  $(SDK_ROOT)/components/libraries/scheduler/app_scheduler.c \
  $(SDK_ROOT)/components/libraries/low_power_pwm/low_power_pwm.c \
  $(SDK_ROOT)/components/drivers_nrf/clock/nrf_drv_clock.c \
  $(SDK_ROOT)/components/drivers_nrf/common/nrf_drv_common.c \
  $(SDK_ROOT)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c \
  $(SDK_ROOT)/components/drivers_nrf/uart/nrf_drv_uart.c \
  $(SDK_ROOT)/components/drivers_nrf/timer/nrf_drv_timer.c \
  $(SDK_ROOT)/components/drivers_nrf/twi_master/nrf_drv_twi.c \
  $(SDK_ROOT)/components/ant/ant_channel_config/ant_channel_config.c \
  $(SDK_ROOT)/components/ant/ant_key_manager/ant_key_manager.c \
  $(SDK_ROOT)/components/ant/ant_stack_config/ant_stack_config.c \
  $(SDK_ROOT)/components/libraries/bsp/bsp.c \
  $(SDK_ROOT)/components/libraries/bsp/bsp_btn_ant.c \
  $(SDK_ROOT)/components/libraries/bsp/bsp_nfc.c \
  $(PROJ_DIR)/src/main.c \
  $(PROJ_DIR)/src/wireless/ant/mm_ant_control.c \
  $(PROJ_DIR)/src/wireless/ant/mm_ant_page_manager.c \
  $(PROJ_DIR)/src/wireless/blaze/mm_blaze_control.c \
  $(PROJ_DIR)/src/protocols/mm_node_config.c \
  $(PROJ_DIR)/src/protocols/mm_switch_config.c \
  $(PROJ_DIR)/src/protocols/mm_led_control.c \
  $(PROJ_DIR)/src/sensors/pir/pir_st_00081.c \
  $(PROJ_DIR)/src/sensors/lidar/lidar.c \
  $(PROJ_DIR)/src/sensors/mm_hardware_test.c \
  $(PROJ_DIR)/src/sensor_management/mm_sensor_manager.c \
  $(PROJ_DIR)/src/sensor_management/mm_sensor_transmission.c \
  $(PROJ_DIR)/src/peripherals/mm_rgb_led.c \
  $(PROJ_DIR)/src/peripherals/mm_power_bank_timer.c \
  $(SDK_ROOT)/external/segger_rtt/RTT_Syscalls_GCC.c \
  $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT.c \
  $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_printf.c \
  $(SDK_ROOT)/components/toolchain/gcc/gcc_startup_nrf52.S \
  $(SDK_ROOT)/components/toolchain/system_nrf52.c \
  $(SDK_ROOT)/components/softdevice/common/softdevice_handler/softdevice_handler.c \

#gateway only source
ifeq ($(IS_BLAZE_GATEWAY),1)
SRC_FILES += \
  $(PROJ_DIR)/src/sensor_algorithm/mm_sensor_algorithm.c \
  $(PROJ_DIR)/src/sensor_algorithm/mm_activity_variables.c \
  $(PROJ_DIR)/src/sensor_algorithm/mm_sensor_error_check.c \
  $(PROJ_DIR)/src/sensor_algorithm/mm_activity_variable_drain.c \
  $(PROJ_DIR)/src/sensor_algorithm/mm_led_strip_states.c \
  $(PROJ_DIR)/src/sensor_algorithm/activity_variable_growth/mm_activity_variable_growth.c \
  $(PROJ_DIR)/src/sensor_algorithm/activity_variable_growth/mm_activity_variable_growth_lidar.c \
  $(PROJ_DIR)/src/sensor_algorithm/activity_variable_growth/mm_activity_variable_growth_pir.c \
  $(PROJ_DIR)/src/sensor_algorithm/activity_variable_growth/mm_activity_variable_growth_sensor_records.c \
  $(PROJ_DIR)/src/protocols/mm_monitoring_dispatch.c \
  $(PROJ_DIR)/src/protocols/mm_position_config.c \
  $(PROJ_DIR)/src/protocols/mm_av_transmission.c \
endif

# Include folders common to all targets
INC_FOLDERS += \
  $(SDK_ROOT)/external/ANT_BLAZE_Libraries_v1.0.0/inc \
  $(SDK_ROOT)/components/libraries/sensorsim \
  $(SDK_ROOT)/components/ant/ant_key_manager \
  $(SDK_ROOT)/components/libraries/hardfault/nrf52 \
  $(SDK_ROOT)/components/softdevice/s332/headers/nrf52 \
  $(SDK_ROOT)/components/toolchain/gcc \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_hrm \
  $(SDK_ROOT)/components/drivers_nrf/uart \
  $(SDK_ROOT)/components/device \
  $(SDK_ROOT)/components/libraries/scheduler \
  $(SDK_ROOT)/components/drivers_nrf/hal \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_hrm/utils \
  $(SDK_ROOT)/components/libraries/strerror \
  $(SDK_ROOT)/components/boards \
  $(SDK_ROOT)/components/drivers_nrf/gpiote \
  $(SDK_ROOT)/components/softdevice/s332/headers \
  $(SDK_ROOT)/components/libraries/button \
  $(SDK_ROOT)/components/ant/ant_state_indicator \
  $(SDK_ROOT)/components/libraries/log \
  $(SDK_ROOT)/components/libraries/log/src \
  $(SDK_ROOT)/components/toolchain \
  $(SDK_ROOT)/components/libraries/bsp \
  $(SDK_ROOT)/components/ant/ant_key_manager/config \
  $(SDK_ROOT)/components/drivers_nrf/clock \
  $(SDK_ROOT)/components/drivers_nrf/timer \
  $(SDK_ROOT)/components/drivers_nrf/twi_master \
  $(SDK_ROOT)/components/libraries/low_power_pwm \
  $(PROJ_DIR)/src/sensors/pir \
  $(PROJ_DIR)/src/sensors/lidar \
  $(PROJ_DIR)/src/sensors \
  $(PROJ_DIR)/src/sensor_management/ \
  $(PROJ_DIR)/src/sensor_algorithm/ \
  $(PROJ_DIR)/src/sensor_algorithm/activity_variable_growth \
  ../config \
  $(SDK_ROOT)/components/libraries/hardfault \
  $(SDK_ROOT)/components/libraries/util \
  $(SDK_ROOT)/components/drivers_nrf/common \
  $(SDK_ROOT)/components \
  $(SDK_ROOT)/components/softdevice/common/softdevice_handler \
  $(SDK_ROOT)/external/segger_rtt \
  $(SDK_ROOT)/components/ant/ant_channel_config \
  $(SDK_ROOT)/components/drivers_nrf/delay \
  $(SDK_ROOT)/components/ant/ant_stack_config \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/components/libraries/timer \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_hrm/pages \
  $(PROJ_DIR)/src/wireless/ant \
  $(PROJ_DIR)/src/wireless/blaze \
  $(PROJ_DIR)/src/protocols \
  $(PROJ_DIR)/src/peripherals \

# Libraries
ifeq ($(IS_BLAZE_GATEWAY),1)
  LIB_FILES += $(SDK_ROOT)/external/ANT_BLAZE_Libraries_v1.0.0/bin/ANT_BLAZE_Gateway_Library_GCC.a
else
  LIB_FILES += $(SDK_ROOT)/external/ANT_BLAZE_Libraries_v1.0.0/bin/ANT_BLAZE_Node_Library_GCC.a
endif


