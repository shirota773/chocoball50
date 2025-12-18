# ⚡ HyperChat Flow

HyperChatと互換性のある、YouTubeライブのコメントを動画上に流す（ニコニコ動画風）Chrome拡張機能です。

## 🌟 特徴

- **HyperChat互換**: HyperChatを有効化しても正常に動作
- **軽量**: パフォーマンスを重視した設計
- **カスタマイズ可能**: 文字サイズ、速度、透明度などを自由に調整
- **スーパーチャット対応**: スーパーチャットも特別な表示で流れる
- **アバター表示**: コメント投稿者のアバターを表示可能

## 📋 動作要件

- Google Chrome 88以上（Manifest V3対応）
- Microsoft Edge 88以上
- HyperChat拡張機能（推奨）

## 🚀 インストール方法

### 開発者モードでインストール（推奨）

1. このリポジトリをダウンロードまたはクローン:
   ```bash
   git clone https://github.com/your-username/hyperchat-flow-extension.git
   ```

2. Chromeを開いて `chrome://extensions/` にアクセス

3. 右上の「デベロッパーモード」を有効化

4. 「パッケージ化されていない拡張機能を読み込む」をクリック

5. `hyperchat-flow-extension` フォルダを選択

6. アイコンを作成（オプション）:
   - `icons/README.md` の手順に従ってPNGアイコンを生成
   - アイコンがなくても動作します

### Chrome Web Storeから（将来的に）

*現在準備中*

## 📖 使い方

1. YouTubeのライブ配信ページを開く

2. 拡張機能アイコンをクリックして設定パネルを開く

3. 以下の項目を調整:
   - **コメントフローを有効化**: オン/オフの切り替え
   - **文字サイズ**: 12px～48px
   - **流れる速度**: 1～15（数字が大きいほど速い）
   - **透明度**: 30%～100%
   - **最大表示メッセージ数**: 10～100
   - **アバターを表示**: アバターの表示/非表示

4. 設定は自動的に保存され、すぐに反映されます

## 🔧 技術詳細

### アーキテクチャ

```
hyperchat-flow-extension/
├── manifest.json          # 拡張機能の設定
├── content.js            # メインのコメントフロー機能
├── style.css             # コメント表示のスタイル
├── popup.html            # 設定画面のUI
├── popup.js              # 設定画面のロジック
└── icons/                # アイコン画像
    ├── icon.svg          # SVGアイコン（元ファイル）
    └── README.md         # アイコン生成方法
```

### 動作原理

1. **チャットメッセージの検出**:
   - MutationObserverを使用してチャットコンテナを監視
   - HyperChatと標準YouTubeチャット両方のDOM構造に対応
   - 新しいメッセージが追加されたらリアルタイムで検出

2. **メッセージの抽出**:
   - 通常のテキストメッセージ
   - スーパーチャット
   - HyperChat独自の構造

3. **コメントフローの実装**:
   - 動画コンテナ上にオーバーレイを作成
   - requestAnimationFrameで滑らかなアニメーション
   - 右から左へメッセージを流す

4. **設定管理**:
   - Chrome Storage APIで設定を保存
   - リアルタイムで設定を反映

### 対応しているDOM構造

```javascript
// HyperChat
'yt-live-chat-item-list-renderer'
'#items.yt-live-chat-item-list-renderer'
'[class*="hyperchat"]'

// 標準YouTubeチャット
'yt-live-chat-text-message-renderer'
'yt-live-chat-paid-message-renderer'
```

## 🐛 トラブルシューティング

### コメントが流れない場合

1. **ページを再読み込み**: F5キーまたはCtrl+R
2. **拡張機能を再読み込み**: `chrome://extensions/` で「再読み込み」をクリック
3. **設定を確認**: 「コメントフローを有効化」がオンになっているか確認
4. **ブラウザコンソールを確認**: F12でデベロッパーツールを開き、エラーメッセージを確認

### HyperChatと互換性がない場合

1. HyperChatとHyperChat Flowの両方が最新版か確認
2. ブラウザのキャッシュをクリア
3. 両方の拡張機能を無効化してから、順番に有効化

### パフォーマンスの問題

1. **最大表示メッセージ数を減らす**: 設定で30以下に設定
2. **文字サイズを小さくする**: 大きな文字は処理が重い
3. **アバター表示をオフ**: アバターの読み込みが重い場合

## 🤝 貢献

バグ報告や機能リクエストは、GitHubのIssuesで受け付けています。

プルリクエストも歓迎します！

## 📝 ライセンス

MIT License

## 🙏 クレジット

- [HyperChat](https://github.com/LiveTL/HyperChat) - YouTubeチャットの最適化
- [youtube-live-chat-flow](https://github.com/tsukumijima/youtube-live-chat-flow) - コメントフローのインスピレーション

## 📮 お問い合わせ

- GitHub Issues: [Create an issue](https://github.com/your-username/hyperchat-flow-extension/issues)
- Twitter: [@your_twitter](https://twitter.com/your_twitter)

---

Made with ❤️ for YouTube Live
