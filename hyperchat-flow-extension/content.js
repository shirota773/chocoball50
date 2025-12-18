// HyperChat Flow - Content Script
// YouTubeライブのコメントを動画上に流す

class HyperChatFlow {
  constructor() {
    this.config = {
      enabled: true,
      fontSize: 24,
      speed: 5,
      opacity: 0.8,
      showAvatar: true,
      maxMessages: 50
    };
    this.messages = [];
    this.overlay = null;
    this.videoContainer = null;
    this.observer = null;
    this.animationFrame = null;

    this.init();
  }

  async init() {
    // 設定を読み込む
    await this.loadConfig();

    // 動画コンテナの準備ができるまで待つ
    this.waitForVideo();
  }

  async loadConfig() {
    try {
      const stored = await chrome.storage.sync.get('config');
      if (stored.config) {
        this.config = { ...this.config, ...stored.config };
      }
    } catch (error) {
      console.log('HyperChat Flow: Using default config');
    }
  }

  waitForVideo() {
    const checkVideo = setInterval(() => {
      const video = document.querySelector('video.html5-main-video');
      const container = document.querySelector('.html5-video-container');

      if (video && container) {
        clearInterval(checkVideo);
        this.videoContainer = container;
        this.setupOverlay();
        this.observeChatMessages();
        console.log('HyperChat Flow: Initialized');
      }
    }, 500);
  }

  setupOverlay() {
    // オーバーレイコンテナを作成
    this.overlay = document.createElement('div');
    this.overlay.id = 'hyperchat-flow-overlay';
    this.overlay.className = 'hyperchat-flow-overlay';
    this.videoContainer.appendChild(this.overlay);

    // リサイズイベントの監視
    window.addEventListener('resize', () => this.handleResize());
  }

  observeChatMessages() {
    // HyperChatとYouTubeの標準チャット両方に対応
    const chatSelectors = [
      // HyperChatのセレクタ（Svelteベース）
      'yt-live-chat-item-list-renderer',
      '#item-scroller',
      '#items.yt-live-chat-item-list-renderer',
      // 標準YouTubeチャット
      '#chat-messages',
      '#item-list',
      // HyperChatの特定のコンテナ
      '[class*="hyperchat"]',
      '[id*="hyperchat"]'
    ];

    // 複数のセレクタを試す
    let chatContainer = null;
    for (const selector of chatSelectors) {
      chatContainer = document.querySelector(selector);
      if (chatContainer) {
        console.log(`HyperChat Flow: Found chat container with selector: ${selector}`);
        break;
      }
    }

    if (!chatContainer) {
      // チャットコンテナが見つからない場合、後で再試行
      console.log('HyperChat Flow: Chat container not found, retrying...');
      setTimeout(() => this.observeChatMessages(), 1000);
      return;
    }

    // MutationObserverでチャットメッセージの追加を監視
    this.observer = new MutationObserver((mutations) => {
      mutations.forEach((mutation) => {
        mutation.addedNodes.forEach((node) => {
          if (node.nodeType === Node.ELEMENT_NODE) {
            this.processNewMessage(node);
          }
        });
      });
    });

    this.observer.observe(chatContainer, {
      childList: true,
      subtree: true
    });

    // 既存のメッセージを処理
    const existingMessages = chatContainer.querySelectorAll('yt-live-chat-text-message-renderer, yt-live-chat-paid-message-renderer');
    existingMessages.forEach(msg => this.processNewMessage(msg));
  }

  processNewMessage(node) {
    if (!this.config.enabled) return;

    let messageData = null;

    // 通常のテキストメッセージ
    if (node.tagName === 'YT-LIVE-CHAT-TEXT-MESSAGE-RENDERER' ||
        node.classList?.contains('yt-live-chat-text-message-renderer')) {
      messageData = this.extractTextMessage(node);
    }
    // スーパーチャット
    else if (node.tagName === 'YT-LIVE-CHAT-PAID-MESSAGE-RENDERER' ||
             node.classList?.contains('yt-live-chat-paid-message-renderer')) {
      messageData = this.extractPaidMessage(node);
    }
    // HyperChatの特定の構造を検出
    else if (node.querySelector('#message, [id*="message"]')) {
      messageData = this.extractHyperChatMessage(node);
    }

    if (messageData) {
      this.addFlowingMessage(messageData);
    }
  }

  extractTextMessage(node) {
    try {
      const authorElement = node.querySelector('#author-name');
      const messageElement = node.querySelector('#message, yt-live-chat-text-message-renderer #message');
      const avatarElement = node.querySelector('#img');

      if (!messageElement) return null;

      return {
        author: authorElement?.textContent?.trim() || 'Anonymous',
        message: messageElement.textContent?.trim() || '',
        avatar: avatarElement?.src || null,
        color: this.getRandomColor(),
        isPaid: false
      };
    } catch (error) {
      return null;
    }
  }

  extractPaidMessage(node) {
    try {
      const authorElement = node.querySelector('#author-name');
      const messageElement = node.querySelector('#message, #purchase-amount-column');
      const avatarElement = node.querySelector('#img');

      return {
        author: authorElement?.textContent?.trim() || 'Anonymous',
        message: messageElement?.textContent?.trim() || 'Super Chat!',
        avatar: avatarElement?.src || null,
        color: '#ffd700',
        isPaid: true
      };
    } catch (error) {
      return null;
    }
  }

  extractHyperChatMessage(node) {
    try {
      // HyperChatの構造を推測して抽出
      const authorElement = node.querySelector('[class*="author"], [id*="author"]');
      const messageElement = node.querySelector('[class*="message"], [id*="message"]');
      const avatarElement = node.querySelector('img');

      if (!messageElement) return null;

      return {
        author: authorElement?.textContent?.trim() || 'Anonymous',
        message: messageElement.textContent?.trim() || '',
        avatar: avatarElement?.src || null,
        color: this.getRandomColor(),
        isPaid: false
      };
    } catch (error) {
      return null;
    }
  }

  addFlowingMessage(data) {
    const messageElement = document.createElement('div');
    messageElement.className = 'flow-message';
    if (data.isPaid) {
      messageElement.classList.add('paid-message');
    }

    // メッセージコンテンツを作成
    let content = '';
    if (this.config.showAvatar && data.avatar) {
      content += `<img src="${data.avatar}" class="flow-avatar" alt="avatar">`;
    }
    content += `<span class="flow-text">${this.escapeHtml(data.message)}</span>`;

    messageElement.innerHTML = content;
    messageElement.style.fontSize = `${this.config.fontSize}px`;
    messageElement.style.opacity = this.config.opacity;
    messageElement.style.color = data.color;

    // ランダムな垂直位置を設定
    const videoHeight = this.videoContainer.offsetHeight;
    const randomTop = Math.random() * (videoHeight - 50);
    messageElement.style.top = `${randomTop}px`;

    // オーバーレイに追加
    this.overlay.appendChild(messageElement);

    // アニメーション開始
    this.animateMessage(messageElement);

    // 最大メッセージ数を超えたら古いものを削除
    this.messages.push(messageElement);
    if (this.messages.length > this.config.maxMessages) {
      const oldMessage = this.messages.shift();
      if (oldMessage && oldMessage.parentNode) {
        oldMessage.remove();
      }
    }
  }

  animateMessage(element) {
    const containerWidth = this.videoContainer.offsetWidth;
    const messageWidth = element.offsetWidth;

    // 右端から左端まで移動
    let position = containerWidth;
    const speed = this.config.speed;

    const animate = () => {
      position -= speed;
      element.style.transform = `translateX(${position}px)`;

      if (position > -messageWidth) {
        requestAnimationFrame(animate);
      } else {
        // アニメーション終了後、要素を削除
        element.remove();
        const index = this.messages.indexOf(element);
        if (index > -1) {
          this.messages.splice(index, 1);
        }
      }
    };

    animate();
  }

  getRandomColor() {
    const colors = [
      '#FFFFFF', '#FF6B6B', '#4ECDC4', '#45B7D1',
      '#FFA07A', '#98D8C8', '#F7DC6F', '#BB8FCE',
      '#85C1E2', '#F8B739'
    ];
    return colors[Math.floor(Math.random() * colors.length)];
  }

  escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
  }

  handleResize() {
    // リサイズ時の処理（必要に応じて）
  }

  destroy() {
    if (this.observer) {
      this.observer.disconnect();
    }
    if (this.overlay) {
      this.overlay.remove();
    }
    if (this.animationFrame) {
      cancelAnimationFrame(this.animationFrame);
    }
  }
}

// ページロード時に初期化
if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', () => {
    window.hyperChatFlow = new HyperChatFlow();
  });
} else {
  window.hyperChatFlow = new HyperChatFlow();
}

// 設定更新のリスナー
chrome.storage.onChanged.addListener((changes, namespace) => {
  if (namespace === 'sync' && changes.config) {
    if (window.hyperChatFlow) {
      window.hyperChatFlow.config = { ...window.hyperChatFlow.config, ...changes.config.newValue };
    }
  }
});
