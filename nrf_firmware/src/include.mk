#control constants
IS_BLAZE_GATEWAY = 0

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
  $(SDK_ROOT)/components/libraries/pwm/app_pwm.c \
  $(SDK_ROOT)/components/drivers_nrf/clock/nrf_drv_clock.c \
  $(SDK_ROOT)/components/drivers_nrf/common/nrf_drv_common.c \
  $(SDK_ROOT)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c \
  $(SDK_ROOT)/components/drivers_nrf/uart/nrf_drv_uart.c \
  $(SDK_ROOT)/components/drivers_nrf/timer/nrf_drv_timer.c \
  $(SDK_ROOT)/components/drivers_nrf/ppi/nrf_drv_ppi.c \
  $(SDK_ROOT)/components/ant/ant_channel_config/ant_channel_config.c \
  $(SDK_ROOT)/components/ant/ant_key_manager/ant_key_manager.c \
  $(SDK_ROOT)/components/ant/ant_stack_config/ant_stack_config.c \
  $(SDK_ROOT)/components/libraries/bsp/bsp.c \
  $(SDK_ROOT)/components/libraries/bsp/bsp_btn_ant.c \
  $(SDK_ROOT)/components/libraries/bsp/bsp_nfc.c \
  $(PROJ_DIR)/src/main.c \
  $(PROJ_DIR)/src/wireless/ant/mm_ant_control.c \
  $(PROJ_DIR)/src/wireless/blaze/mm_blaze_control.c \
  $(PROJ_DIR)/src/protocols/mm_node_config.c \
  $(PROJ_DIR)/src/demo/mm_blaze_basic_txrx_demo.c \
  $(PROJ_DIR)/src/sensors/pir/pir_st_00081.c \
  $(PROJ_DIR)/src/sensors/pir/pir_28027.c \
  $(PROJ_DIR)/src/sensors/ir_led/ky_022_receive.c \
  $(PROJ_DIR)/src/sensors/ir_led/ir_led_transmit.c \
  $(SDK_ROOT)/external/segger_rtt/RTT_Syscalls_GCC.c \
  $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT.c \
  $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_printf.c \
  $(SDK_ROOT)/components/toolchain/gcc/gcc_startup_nrf52.S \
  $(SDK_ROOT)/components/toolchain/system_nrf52.c \
  $(SDK_ROOT)/components/softdevice/common/softdevice_handler/softdevice_handler.c \

# Include folders common to all targets
INC_FOLDERS += \
  $(SDK_ROOT)/external/ANT_BLAZE_Libraries_v1.0.0/inc \
  $(SDK_ROOT)/components/libraries/sensorsim \
  $(SDK_ROOT)/components/ant/ant_key_manager \
  $(SDK_ROOT)/components/libraries/hardfault/nrf52 \
  $(SDK_ROOT)/components/softdevice/s332/headers/nrf52 \
  $(SDK_ROOT)/components/toolchain/gcc \
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
  $(SDK_ROOT)/components/libraries/log \
  $(SDK_ROOT)/components/libraries/log/src \
  $(SDK_ROOT)/components/toolchain \
  $(SDK_ROOT)/components/libraries/bsp \
  $(SDK_ROOT)/components/ant/ant_key_manager/config \
  $(SDK_ROOT)/components/drivers_nrf/clock \
  $(SDK_ROOT)/components/drivers_nrf/ppi \
  $(SDK_ROOT)/components/drivers_nrf/timer \
  $(SDK_ROOT)/components/libraries/pwm \
  $(PROJ_DIR)/src/sensors/pir \
  $(PROJ_DIR)/src/sensors/ir_led \
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
  $(PROJ_DIR)/src/demo \

# Libraries
ifeq ($(IS_BLAZE_GATEWAY),1)
  LIB_FILES += $(SDK_ROOT)/external/ANT_BLAZE_Libraries_v1.0.0/bin/ANT_BLAZE_Gateway_Library_GCC.a
else
  LIB_FILES += $(SDK_ROOT)/external/ANT_BLAZE_Libraries_v1.0.0/bin/ANT_BLAZE_Node_Library_GCC.a
endif

  
  