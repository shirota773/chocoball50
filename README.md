# Chocoball50 ZMK Firmware

4x13 Ortholinear keyboard with trackball and rotary encoder support.

## Features

- **Board**: nice!nano (nRF52840)
- **Layout**: 4x13 Ortholinear (52 keys)
- **Encoder**: Horizontal rotary encoder
- **Lighting**: WS2812 RGB underglow LEDs
- **Connectivity**: Bluetooth 5.0 with USB fallback
- **Battery**: Supports rechargeable lithium battery (nice!nano feature)
- **Trackball**: az1uball (PMW3360 sensor) - *I2C-based trackball support*

## Building Firmware

### GitHub Actions (Recommended)

Push to the repository and GitHub Actions will automatically build the firmware. Download the `.uf2` file from the Actions artifacts.

### Local Build

```bash
west init -l config
west update
west build -s zmk/app -b nice_nano -- -DSHIELD=chocoball50
```

## Flashing

1. Connect nice!nano via USB
2. Double-tap reset button to enter bootloader mode
3. Copy the `.uf2` file to the mounted drive (NICENANO)

## Keymap

- **Layer 0 (Default)**: Standard QWERTY layout
- **Layer 1 (Lower)**: Numbers, F-keys, navigation
- **Layer 2 (Raise)**: Symbols
- **Layer 3 (Adjust)**: Bluetooth, RGB controls, system functions

## Configuration Files

- `config/boards/shields/chocoball50/chocoball50.overlay` - Hardware definition
- `config/boards/shields/chocoball50/chocoball50.keymap` - Key mappings
- `config/boards/shields/chocoball50/chocoball50.conf` - Firmware settings

## Pin Mapping (nice!nano)

### Matrix (4 rows x 13 columns)
- **Rows**: D1 (TX), D0 (RX), D2, D3 (Pro Micro pins 0-3)
- **Columns**: D4-D10, D14-D16, A0-A2 (Pro Micro pins 4-10, 14-16, 18-20)

### Rotary Encoder
- **A**: P1.01 (Extra GPIO)
- **B**: P1.02 (Extra GPIO)

### Trackball (I2C)
- **Bus**: I2C0 (uses D0/D1 by default)
- **Address**: 0x0A

### LED Strip (WS2812)
- **Data**: P1.07 (Extra GPIO, via SPI3)
- **Power Control**: P1.13 (configurable based on hardware)

**Note**: nice!nano uses Pro Micro compatible pinout. Extra GPIO pins (P1.01, P1.02, P1.07) are available on the back of the board.

## Customization

Edit `chocoball50.keymap` to customize your key layout. Edit `chocoball50.conf` for firmware settings.

## Trackball Support

The firmware includes custom I2C-based trackball support (az1uball driver) for PMW3360 sensor. The trackball is configured to:

- Poll at 8ms intervals (125Hz)
- Support axis inversion and swapping
- Report button state
- Thread priority set to 50

The trackball configuration can be adjusted in `chocoball50.conf`:
- `CONFIG_AZ1UBALL_POLL_INTERVAL_MS`: Polling interval (default: 8)
- `CONFIG_AZ1UBALL_THREAD_PRIORITY`: Thread priority (default: 50)

To adjust trackball behavior, modify the devicetree properties in `chocoball50.overlay`:
- `invert-x`: Invert X-axis movement
- `invert-y`: Invert Y-axis movement
- `swap-xy`: Swap X and Y axes

## License

MIT License
