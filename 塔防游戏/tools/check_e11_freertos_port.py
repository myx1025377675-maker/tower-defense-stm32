from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def read_text(path):
    return path.read_text(encoding="utf-8", errors="ignore")


def check(condition, message, failures):
    status = "OK" if condition else "FAIL"
    print(f"[{status}] {message}")
    if not condition:
        failures.append(message)


def main():
    failures = []

    project = read_text(ROOT / "MDK-ARM" / "FK743M2.uvprojx")
    main_c = read_text(ROOT / "Core" / "Src" / "main.c")
    it_c = read_text(ROOT / "Core" / "Src" / "stm32h7xx_it.c")
    lcd_rgb_c = read_text(ROOT / "Drivers" / "User" / "Src" / "lcd_rgb.c")
    lcd_rgb_h = read_text(ROOT / "Drivers" / "User" / "Inc" / "lcd_rgb.h")
    lcd_pwm_h = read_text(ROOT / "Drivers" / "User" / "Inc" / "lcd_pwm.h")
    lcd_pwm_c = read_text(ROOT / "Drivers" / "User" / "Src" / "lcd_pwm.c")
    touch_iic_h = read_text(ROOT / "Drivers" / "User" / "Inc" / "touch_iic.h")
    e08_lab_c = read_text(ROOT / "Drivers" / "User" / "Src" / "e08_lab.c")

    check((ROOT / "Core" / "Inc" / "FreeRTOSConfig.h").exists(), "FreeRTOSConfig.h exists", failures)
    check("FreeRTOS-Kernel-10.4.6" in project, "Keil project references FreeRTOS kernel paths", failures)
    check(
        r"portable\GCC\ARM_CM7\r0p1\port.c" in project,
        "Keil project uses GCC ARM_CM7 port (ARM Compiler 6 / ARMClang)",
        failures,
    )
    check(
        r"portable\RVDS\ARM_CM7\r0p1\port.c" not in project,
        "Keil project does not use RVDS ARM_CM7 port (incompatible with ARMClang)",
        failures,
    )
    check("tasks.c" in project and "queue.c" in project and "port.c" in project and "heap_4.c" in project,
          "Keil project includes core FreeRTOS source files", failures)
    check("e11_freertos_app.c" in project, "Keil project includes E11 app source", failures)
    check('#include "e11_freertos_app.h"' in main_c and "E11_FreeRTOS_Start();" in main_c,
          "main starts E11 FreeRTOS app", failures)

    check("xPortSysTickHandler" in it_c and "xTaskGetSchedulerState" in it_c,
          "SysTick forwards to FreeRTOS after scheduler starts", failures)
    check("void SVC_Handler(void)" not in it_c, "stm32h7xx_it.c no longer defines SVC_Handler", failures)
    check("void PendSV_Handler(void)" not in it_c, "stm32h7xx_it.c no longer defines PendSV_Handler", failures)

    check("PG11    ------> LTDC_B3" in lcd_rgb_c, "LCD_B3 comment documents PG11", failures)
    check("GPIO_PIN_11" in lcd_rgb_c and "HAL_GPIO_Init(GPIOG" in lcd_rgb_c,
          "LCD_B3 is configured on PG11", failures)
    check("GPIO_AF13_LTDC" not in lcd_rgb_c, "old PA8 AF13 LTDC configuration removed", failures)

    check("LCD_BL_YUANZI_WIRING" in lcd_rgb_h, "LCD backlight: Fanke PH6 / Yuanzi PB5 switch exists", failures)
    check(
        "GPIO_LDC_Backlight_CLK_ENABLE" in lcd_rgb_h and "LCD_Backlight_PIN" in lcd_rgb_h,
        "LCD backlight GPIO macros present",
        failures,
    )
    check("TIM4->CCR4" not in lcd_pwm_c, "LCD_PwmSetPulse no longer writes TIM4 CCR4", failures)
    check("TIM12" in lcd_pwm_h and "TIM3" in lcd_pwm_h,
          "LCD PWM header supports Fanke TIM12 and Yuanzi TIM3", failures)

    check("Touch_RST_PORT" in touch_iic_h and "GPIOH" in touch_iic_h and "GPIO_PIN_4" in touch_iic_h,
          "touch reset is PH4", failures)
    check("Touch_INT_PORT" in touch_iic_h and "GPIOG" in touch_iic_h and "GPIO_PIN_3" in touch_iic_h,
          "touch interrupt is PG3", failures)
    check("Touch_IIC_SCL_PORT" in touch_iic_h and "GPIOI" in touch_iic_h and "GPIO_PIN_11" in touch_iic_h,
          "touch SCL is PI11", failures)
    check("Touch_IIC_SDA_PORT" in touch_iic_h and "GPIOI" in touch_iic_h and "GPIO_PIN_8" in touch_iic_h,
          "touch SDA is PI8", failures)

    check("vTaskDelay(pdMS_TO_TICKS(1))" in e08_lab_c,
          "LCD lyrics loop yields while waiting for next frame", failures)

    if failures:
        print(f"\n{len(failures)} check(s) failed.")
        raise SystemExit(1)

    print("\nAll E11 FreeRTOS port checks passed.")


if __name__ == "__main__":
    main()
