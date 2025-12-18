# アイコンについて

このディレクトリには拡張機能のアイコンが必要です。

## 必要なアイコン

以下のサイズのPNGファイルを作成してください：

- `icon16.png` (16x16ピクセル)
- `icon48.png` (48x48ピクセル)
- `icon128.png` (128x128ピクセル)

## SVGからPNGへの変換

`icon.svg`ファイルが用意されています。以下の方法でPNGに変換できます：

### オンラインツールを使用
- https://cloudconvert.com/svg-to-png
- https://www.aconvert.com/image/svg-to-png/

### コマンドラインツールを使用（ImageMagickがインストールされている場合）

```bash
# 16x16
convert -background none -resize 16x16 icon.svg icon16.png

# 48x48
convert -background none -resize 48x48 icon.svg icon48.png

# 128x128
convert -background none -resize 128x128 icon.svg icon128.png
```

### Inkscapeを使用

```bash
inkscape icon.svg --export-filename=icon16.png -w 16 -h 16
inkscape icon.svg --export-filename=icon48.png -w 48 -h 48
inkscape icon.svg --export-filename=icon128.png -w 128 -h 128
```

## 一時的な対処法

アイコンがなくてもブラウザ拡張機能は動作します。Chromeがデフォルトのアイコンを表示します。
