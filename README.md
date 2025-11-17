# Chocoball50 ZMK Firmware

4x13 Ortholinear keyboard with trackball and rotary encoder support.

## Features

- **Board**: Seeed XIAO BLE (nRF52840)
- **Layout**: 4x13 Ortholinear (52 keys)
- **Encoder**: Horizontal rotary encoder
- **Lighting**: WS2812 RGB underglow LEDs
- **Connectivity**: Bluetooth 5.0 with USB fallback
- **Trackball**: az1uball (PMW3360 sensor) - *Planned, requires custom driver*

## Building Firmware

### GitHub Actions (Recommended)

Push to the repository and GitHub Actions will automatically build the firmware. Download the `.uf2` file from the Actions artifacts.

### Local Build

```bash
west init -l config
west update
west build -s zmk/app -b seeeduino_xiao_ble -- -DSHIELD=chocoball50
```

## Flashing

1. Connect XIAO BLE via USB
2. Double-tap reset button to enter bootloader mode
3. Copy the `.uf2` file to the mounted drive

## Keymap

- **Layer 0 (Default)**: Standard QWERTY layout
- **Layer 1 (Lower)**: Numbers, F-keys, navigation
- **Layer 2 (Raise)**: Symbols
- **Layer 3 (Adjust)**: Bluetooth, RGB controls, system functions

## Configuration Files

- `config/boards/shields/chocoball50/chocoball50.overlay` - Hardware definition
- `config/boards/shields/chocoball50/chocoball50.keymap` - Key mappings
- `config/boards/shields/chocoball50/chocoball50.conf` - Firmware settings

## Pin Mapping

### Matrix (4 rows x 13 columns)
- Rows: D0, D1, D2, D3
- Columns: D4-D10, P0.02, P0.03, P0.28, P0.29, P0.30, P0.31

### Rotary Encoder
- A: P0.04
- B: P0.05

### Trackball (SPI)
- CS: P0.17
- IRQ: P0.20

### LED Strip
- Data: P0.06

## Customization

Edit `chocoball50.keymap` to customize your key layout. Edit `chocoball50.conf` for firmware settings.

## Trackball Support

Trackball support (az1uball with PMW3360 sensor) is planned but currently disabled as it requires a custom ZMK driver module. The hardware configuration is prepared in the overlay file (commented out). To enable trackball support in the future:

1. Add PMW3360 driver module to ZMK
2. Uncomment trackball sections in `chocoball50.overlay`
3. Uncomment trackball configs in `Kconfig.defconfig` and `chocoball50.conf`

## License

MIT License
