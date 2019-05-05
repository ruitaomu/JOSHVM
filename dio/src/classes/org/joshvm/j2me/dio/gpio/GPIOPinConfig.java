/*
 * Copyright (C) Max Mu
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Please visit www.joshvm.org if you need additional information or
 * have any questions.
 */
package org.joshvm.j2me.dio.gpio;

import org.joshvm.j2me.dio.DeviceConfig;

public final class GPIOPinConfig implements  DeviceConfig, DeviceConfig.HardwareAddressing {
    /**
     * Bidirectional pin direction with initial input direction.
     */
    public static final int DIR_BOTH_INIT_INPUT = 2;
    /**
     * Bidirectional pin direction with initial output direction.
     */
    public static final int DIR_BOTH_INIT_OUTPUT = 3;
    /**
     * Input pin direction.
     */
    public static final int DIR_INPUT_ONLY = 0;
    /**
     * Output pin direction.
     */
    public static final int DIR_OUTPUT_ONLY = 1;
    /**
     * Input pull-down drive mode.
     * <p>
     * This bit flag can be bitwise-combined (OR) with other drive mode bit flags.
     * </p>
     */
    public static final int MODE_INPUT_PULL_DOWN = 2;
    /**
     * Input pull-up drive mode.
     * <p>
     * This bit flag can be bitwise-combined (OR) with other drive mode bit flags.
     * </p>
     */
    public static final int MODE_INPUT_PULL_UP = 1;
    /**
     * Output open-drain drive mode.
     * <p>
     * This bit flag can be bitwise-combined (OR) with other drive mode bit flags.
     * </p>
     */
    public static final int MODE_OUTPUT_OPEN_DRAIN = 8;
    /**
     * Output push-pull drive mode.
     * <p>
     * This bit flag can be bitwise-combined (OR) with other drive mode bit flags.
     * </p>
     */
    public static final int MODE_OUTPUT_PUSH_PULL = 4;
    /**
     * Rising edge trigger.
     */
    public static final int TRIGGER_BOTH_EDGES = 3;
    /**
     * Both levels trigger.
     */
    public static final int TRIGGER_BOTH_LEVELS = 6;
    /**
     * Falling edge trigger.
     */
    public static final int TRIGGER_FALLING_EDGE = 1;
    /**
     * High level trigger.
     */
    public static final int TRIGGER_HIGH_LEVEL = 4;
    /**
     * Low level trigger.
     */
    public static final int TRIGGER_LOW_LEVEL = 5;
    /**
     * No interrupt trigger.
     */
    public static final int TRIGGER_NONE = 0;
    /**
     * Rising edge trigger.
     */
    public static final int TRIGGER_RISING_EDGE = 2;


	private String controllerName = null;
    private int direction = UNASSIGNED;
    private boolean initValue;
    private int mode = UNASSIGNED;
    private int pinNumber = UNASSIGNED;
    private int controllerNumber = UNASSIGNED;
    private int trigger = UNASSIGNED;

	GPIOPinConfig() {}

	public GPIOPinConfig(int controllerNumber,
				 int pinNumber,
				 int direction,
				 int mode,
				 int trigger,
				 boolean initValue) {
        this.controllerNumber = controllerNumber;
        this.pinNumber = pinNumber;
        if (controllerNumber < DeviceConfig.UNASSIGNED || pinNumber < DeviceConfig.UNASSIGNED ) {
            throw new IllegalArgumentException();
        }
        this.direction = direction;
        this.trigger = trigger;
        this.mode = mode;
        this.initValue = initValue;
        checkAll();

	}
	
	public GPIOPinConfig(java.lang.String controllerName,
             int pinNumber,
             int direction,
             int mode,
             int trigger,
             boolean initValue) {
        this(UNASSIGNED, pinNumber, direction, mode, trigger, initValue);
        this.controllerName = controllerName;
	}

	 /**
     * Gets the configured pin direction.
     *
     * @return the pin direction, one of: {@link #DIR_INPUT_ONLY}, {@link #DIR_OUTPUT_ONLY},
     *         {@link #DIR_BOTH_INIT_INPUT}, {@link #DIR_BOTH_INIT_OUTPUT}.
     */
    public int getDirection() {
        return direction;
    }

    /**
     * Gets the configured pin drive mode.
     *
     * @return the pin drive mode: either {@link #UNASSIGNED UNASSIGNED} or a bitwise OR of at least one of :
     *         {@link #MODE_INPUT_PULL_UP}, {@link #MODE_INPUT_PULL_DOWN},
     *         {@link #MODE_OUTPUT_PUSH_PULL}, {@link #MODE_OUTPUT_OPEN_DRAIN}.
     */
    public int getDriveMode() {
        return mode;
    }

    /**
     * Gets the configured initial value of the pin, if configured for output.
     *
     * @return the pin's initial output value; {@code false} if configured for input.
     */
    public boolean getInitValue() {
        return initValue;
    }

    /**
     * Gets the configured pin number.
     *
     * @return the hardware pin's number (a positive or zero integer) or {@link #UNASSIGNED UNASSIGNED}.
     */
    public int getPinNumber() {
        return pinNumber;
    }

    /**
     * Gets the configured controller number for the pin.
     *
     * @return the hardware port's number (a positive or zero integer) or {@link #UNASSIGNED UNASSIGNED}.
     */
    public int getControllerNumber() {
        return controllerNumber;
    }

    /**
     * Gets the configured controller name (such as its <em>device file</em> name on UNIX systems).
     *
     * @return the controller name or {@code null}.
     */
    public String getControllerName() {
        return controllerName;
    }

    /**
     * Gets the configured initial pin interrupt trigger.
     *
     * @return the pin interrupt trigger, one of: {@link #TRIGGER_NONE},
     *         {@link #TRIGGER_FALLING_EDGE}, {@link #TRIGGER_RISING_EDGE},
     *         {@link #TRIGGER_BOTH_EDGES}, {@link #TRIGGER_HIGH_LEVEL}, {@link #TRIGGER_LOW_LEVEL},
     *         {@link #TRIGGER_BOTH_LEVELS}.
     */
    public int getTrigger() {
        return trigger;
    }

    /**
     * Checks two {@code GPIOPinConfig} objects for equality.
     *
     * @param obj
     *            the object to test for equality with this object.
     * @return {@code true} if {@code obj} is a {@code GPIOPinConfig} and has the same hardware
     *         addressing information and configuration parameter values as this
     *         {@code GPIOPinConfig} object; {@code false} otherwise.
     */
    public boolean equals(Object obj) {
    	if ((obj instanceof GPIOPinConfig) == false) {
			return false;
		}

		GPIOPinConfig cfg = (GPIOPinConfig)obj;
		
        return ((pinNumber == cfg.pinNumber) &&
			    (controllerNumber == cfg.controllerNumber) &&
			    (mode == cfg.mode) &&
			    (trigger == cfg.trigger) &&
			    (direction == cfg.direction) &&
			    (initValue == cfg.initValue)
			    );
    }


    private static void checkDirection(int direction) {
        if (direction < DIR_INPUT_ONLY || direction > DIR_BOTH_INIT_OUTPUT) {
            throw new IllegalArgumentException("GPIO_INVALID_DIRECTION");
        }
    }

    private static void checkMode(int mode) {
        if (UNASSIGNED != mode
            && (0 == mode
                || 0 != (mode & ~(GPIOPinConfig.MODE_INPUT_PULL_UP | GPIOPinConfig.MODE_INPUT_PULL_DOWN | GPIOPinConfig.MODE_OUTPUT_PUSH_PULL | GPIOPinConfig.MODE_OUTPUT_OPEN_DRAIN))
                || (GPIOPinConfig.MODE_INPUT_PULL_UP | GPIOPinConfig.MODE_INPUT_PULL_DOWN) == (mode & (GPIOPinConfig.MODE_INPUT_PULL_UP | GPIOPinConfig.MODE_INPUT_PULL_DOWN))
                || (GPIOPinConfig.MODE_OUTPUT_PUSH_PULL | GPIOPinConfig.MODE_OUTPUT_OPEN_DRAIN) == (mode & (GPIOPinConfig.MODE_OUTPUT_PUSH_PULL | GPIOPinConfig.MODE_OUTPUT_OPEN_DRAIN)))) {
            throw new IllegalArgumentException("GPIO_INVALID_MODE");
        }
    }

    private static void checkTrigger(int trigger) {
        if (trigger < TRIGGER_NONE || trigger > TRIGGER_BOTH_LEVELS) {
            throw new IllegalArgumentException("GPIO_INVALID_TRIGGER");
        }
    }

    /** check if @see GPIOPinConfig.Builder.build() passed successfully.
    * It differs from checking for setters such as setPinNumber etc. */
    private void checkConsistency() {
        if (direction == DIR_OUTPUT_ONLY && trigger != TRIGGER_NONE) {
            throw new IllegalArgumentException("GPIO_INCOMPATIBLE_DIR");
        }

        switch (direction) {
        case GPIOPinConfig.DIR_INPUT_ONLY:
            if (UNASSIGNED != mode &&
                0 != (mode & ~(GPIOPinConfig.MODE_INPUT_PULL_DOWN | GPIOPinConfig.MODE_INPUT_PULL_UP))) {
                throw new IllegalArgumentException("GPIO_INCOMPATIBLE_MODE");
            }
            break;
        case GPIOPinConfig.DIR_OUTPUT_ONLY:
            if (UNASSIGNED != mode &&
                0 != (mode & ~(GPIOPinConfig.MODE_OUTPUT_OPEN_DRAIN | GPIOPinConfig.MODE_OUTPUT_PUSH_PULL ))) {
                throw new IllegalArgumentException("GPIO_INCOMPATIBLE_MODE");
            }
            break;
        case GPIOPinConfig.DIR_BOTH_INIT_INPUT:
        case GPIOPinConfig.DIR_BOTH_INIT_OUTPUT:
            if (UNASSIGNED != mode &&
               ((mode != (MODE_INPUT_PULL_DOWN | MODE_OUTPUT_OPEN_DRAIN)) &&
                (mode != (MODE_INPUT_PULL_DOWN | MODE_OUTPUT_PUSH_PULL)) &&
                (mode != (MODE_INPUT_PULL_UP   | MODE_OUTPUT_OPEN_DRAIN)) &&
                (mode != (MODE_INPUT_PULL_UP   | MODE_OUTPUT_PUSH_PULL))) )  {
                throw new IllegalArgumentException("GPIO_INCOMPATIBLE_MODE");
            }
            break;
        default:
            throw new IllegalArgumentException("GPIO_INCOMPATIBLE_MODE");
        }
    }

    /** check everything for deprecated @see GPIOPinConfig call */
    private void checkAll() {
        checkDirection(direction);
        checkMode(mode);
        checkTrigger(trigger);
        checkConsistency();
    }

}
