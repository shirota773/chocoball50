// HyperChat Flow - Popup Script

document.addEventListener('DOMContentLoaded', async () => {
  // 設定要素を取得
  const elements = {
    enabled: document.getElementById('enabled'),
    fontSize: document.getElementById('fontSize'),
    fontSizeValue: document.getElementById('fontSizeValue'),
    speed: document.getElementById('speed'),
    speedValue: document.getElementById('speedValue'),
    opacity: document.getElementById('opacity'),
    opacityValue: document.getElementById('opacityValue'),
    showAvatar: document.getElementById('showAvatar'),
    maxMessages: document.getElementById('maxMessages'),
    maxMessagesValue: document.getElementById('maxMessagesValue'),
    saveStatus: document.getElementById('saveStatus')
  };

  // デフォルト設定
  const defaultConfig = {
    enabled: true,
    fontSize: 24,
    speed: 5,
    opacity: 0.8,
    showAvatar: true,
    maxMessages: 50
  };

  // 保存された設定を読み込む
  async function loadConfig() {
    try {
      const result = await chrome.storage.sync.get('config');
      const config = result.config || defaultConfig;

      elements.enabled.checked = config.enabled;
      elements.fontSize.value = config.fontSize;
      elements.fontSizeValue.textContent = `${config.fontSize}px`;
      elements.speed.value = config.speed;
      elements.speedValue.textContent = config.speed;
      elements.opacity.value = config.opacity;
      elements.opacityValue.textContent = `${Math.round(config.opacity * 100)}%`;
      elements.showAvatar.checked = config.showAvatar;
      elements.maxMessages.value = config.maxMessages;
      elements.maxMessagesValue.textContent = config.maxMessages;
    } catch (error) {
      console.error('Failed to load config:', error);
    }
  }

  // 設定を保存
  async function saveConfig() {
    const config = {
      enabled: elements.enabled.checked,
      fontSize: parseInt(elements.fontSize.value),
      speed: parseInt(elements.speed.value),
      opacity: parseFloat(elements.opacity.value),
      showAvatar: elements.showAvatar.checked,
      maxMessages: parseInt(elements.maxMessages.value)
    };

    try {
      await chrome.storage.sync.set({ config });
      showSaveStatus();
    } catch (error) {
      console.error('Failed to save config:', error);
    }
  }

  // 保存ステータスを表示
  function showSaveStatus() {
    elements.saveStatus.classList.add('show');
    setTimeout(() => {
      elements.saveStatus.classList.remove('show');
    }, 2000);
  }

  // イベントリスナーを設定
  elements.enabled.addEventListener('change', saveConfig);
  elements.showAvatar.addEventListener('change', saveConfig);

  elements.fontSize.addEventListener('input', (e) => {
    elements.fontSizeValue.textContent = `${e.target.value}px`;
  });
  elements.fontSize.addEventListener('change', saveConfig);

  elements.speed.addEventListener('input', (e) => {
    elements.speedValue.textContent = e.target.value;
  });
  elements.speed.addEventListener('change', saveConfig);

  elements.opacity.addEventListener('input', (e) => {
    elements.opacityValue.textContent = `${Math.round(e.target.value * 100)}%`;
  });
  elements.opacity.addEventListener('change', saveConfig);

  elements.maxMessages.addEventListener('input', (e) => {
    elements.maxMessagesValue.textContent = e.target.value;
  });
  elements.maxMessages.addEventListener('change', saveConfig);

  // 初期化
  await loadConfig();
});
