[arch/arm/boot/dts/stm32mp157-pinctrl.dtsi]
===========================================

			...
            usbotg_fs_dp_dm_pins_a: usbotg-fs-dp-dm-0 {
                pins {
                    pinmux = <STM32_PINMUX('A', 11, ANALOG)>, /* OTG_FS_DM */
                         <STM32_PINMUX('A', 12, ANALOG)>; /* OTG_FS_DP */
                };
            };

            /* added by chunghan.yi@gmail.com -- */
            pinctrl_user2_button_a: user2-pb-0 {
                pins {
                    pinmux = <STM32_PINMUX('A', 14, GPIO)>;
                    drive-push-pull;
                };
            };
            /* -- -- -- */
        };

        pinctrl_z: pin-controller-z@54004000 {
            #address-cells = <1>;
            #size-cells = <1>;
            compatible = "st,stm32mp157-z-pinctrl";
            ranges = <0 0x54004000 0x400>;
            pins-are-numbered;
            interrupt-parent = <&exti>;
            st,syscfg = <&exti 0x60 0xff>;
            hwlocks = <&hsem 0>;
			...


[arch/arm/boot/dts/stm32mp157a-dk1.dts]
=======================================

	...
    led {
        compatible = "gpio-leds";
        blue {
            label = "heartbeat";
            gpios = <&gpiod 11 GPIO_ACTIVE_HIGH>;
            linux,default-trigger = "heartbeat";
            default-state = "off";
        };
    };

    /* added by chunghan.yi@gmail.com, 07/20/2020 -- */
    my_button_interrupt {
        compatible = "eagle,int-button";
/*
        pinctrl-names = "default";
        pinctrl-0 = <&pinctrl_user2_button_a>;
*/
        mybtn-gpios = <&gpioa 14 GPIO_ACTIVE_LOW>;
        interrupt-parent = <&gpioa>;
        interrupts = <14 IRQ_TYPE_EDGE_FALLING>;
    };
    /* -- -- -- */

    sound {
        compatible = "audio-graph-card";
        label = "STM32MP1-DK";
        routing =
            "Playback" , "MCLK",
            "Capture" , "MCLK",
			...

//--------------------------------------------------------------------
