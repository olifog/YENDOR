import './types'
import { setupKeyboardInput, setupMouseInput } from './input'
import {
  initCRT,
  initFramebuffers,
  getGL,
  getSceneFBO,
  renderCRTPass,
  renderPassthrough
} from './renderer'
import type { WasmModule } from './types'
import { audioManager } from './audio'

  // Expose audio function for WASM
  ; (window as any).playGameSound = (id: number) => {
    audioManager.play(id)
  }

  // Expose volume control for WASM
  ; (window as any).setGameVolume = (level: number) => {
    audioManager.setVolume(level);
    localStorage.setItem('game_volume', String(level));
  }

  // Save/Load system
  ; (window as any).saveGameData = (slot: number, code: string, upgrades: string) => {
    const data = {
      code: code,
      upgrades: upgrades,
      timestamp: Date.now()
    };
    localStorage.setItem(`save_slot_${slot}`, JSON.stringify(data));
  }

  ; (window as any).loadGameData = (slot: number): string | null => {
    const data = localStorage.getItem(`save_slot_${slot}`);
    return data;
  }

  ; (window as any).saveSlotExists = (slot: number): number => {
    return localStorage.getItem(`save_slot_${slot}`) ? 1 : 0;
  }

  ; (window as any).getSetting = (key: string): number => {
    const val = localStorage.getItem(`setting_${key}`);
    return val ? parseInt(val) : 0;
  }

  ; (window as any).setSetting = (key: string, value: number) => {
    localStorage.setItem(`setting_${key}`, String(value));
  }

// Menu Art
let menuArtLines: string[] = []
fetch('/ascii_art/menu.txt')
  .then(res => res.text())
  .then(text => {
    // Remove trailing newline if present and split
    menuArtLines = text.replace(/\r\n/g, '\n').split('\n');
  })
  .catch(err => console.error('Failed to load menu art:', err));

; (window as any).getMenuArtLineCount = () => {
  return menuArtLines.length;
}

  ; (window as any).getMenuArtLine = (idx: number): string => {
    if (idx < 0 || idx >= menuArtLines.length) return "";
    return menuArtLines[idx];
  }

// Initialize global window state
window.screenWidth = window.innerWidth
window.screenHeight = window.innerHeight
window.mouseX = 0
window.mouseY = 0
window.mouseDown = 0
window.mouseJustPressed = 0
window.mouseJustReleased = 0
window.scrollDeltaY = 0
window.clipboardText = ''
window.clipboardPasteRequested = 0
window.clipboardCopyRequested = 0
window.selectAllRequested = 0
window.skipCRT = 0

// Value tagging for WASM interop
function tagInt(n: number): number {
  return (n << 1) | 1
}

class Game {
  private canvas: HTMLCanvasElement
  private textCanvas: HTMLCanvasElement
  private wasmModule: WasmModule | null = null
  private lastTime: number = performance.now()

  constructor() {
    this.canvas = document.getElementById('canvas') as HTMLCanvasElement
    this.textCanvas = document.getElementById('textCanvas') as HTMLCanvasElement

    window.textCtx = this.textCanvas.getContext('2d')

    // Hide text canvas - we render it into the WebGL scene
    this.textCanvas.style.display = 'none'

    // Setup input handlers
    setupKeyboardInput(() => this.wasmModule)
    setupMouseInput(this.canvas)
  }

  private resizeCanvas(): void {
    const w = window.innerWidth
    const h = window.innerHeight

    this.canvas.width = w
    this.canvas.height = h
    this.textCanvas.width = w
    this.textCanvas.height = h

    window.screenWidth = w
    window.screenHeight = h

    if (window.textCtx) {
      window.textCtx.font = '14px "Berkeley Mono", monospace'
      window.textCtx.textBaseline = 'top'
    }

    const gl = getGL()
    if (gl) {
      initFramebuffers(w, h)
      gl.viewport(0, 0, w, h)
    }
  }

  private renderFrame = (now: number): void => {
    if (!this.wasmModule) return

    const dt = Math.floor(now - this.lastTime)
    this.lastTime = now

    const gl = getGL()
    const sceneFBO = getSceneFBO()

    if (!gl || !sceneFBO) return

    // Check if we should skip CRT effects (intro/menus)
    const skipCRT = window.skipCRT === 1

    if (skipCRT) {
      // Ensure we're rendering to screen, not to sceneFBO from previous frame
      gl.bindFramebuffer(gl.FRAMEBUFFER, null)
      gl.viewport(0, 0, this.canvas.width, this.canvas.height)

      // Call WASM game functions (which will render to text canvas)
      this.wasmModule._game_update(tagInt(dt))
      this.wasmModule._game_render()
      this.wasmModule._on_frame_start()

      // Render text canvas directly without CRT effects
      renderPassthrough(this.canvas, this.textCanvas)
    } else {
      // 1. Render Game World to FBO
      gl.bindFramebuffer(gl.FRAMEBUFFER, sceneFBO)
      gl.viewport(0, 0, this.canvas.width, this.canvas.height)
      gl.clearColor(0, 0, 0, 1)
      gl.clear(gl.COLOR_BUFFER_BIT)

      // Call WASM game functions
      this.wasmModule._game_update(tagInt(dt))
      this.wasmModule._game_render()
      this.wasmModule._on_frame_start()

      // 2. Composite with CRT shader
      renderCRTPass(this.canvas, this.textCanvas, now)
    }

    // Clear input flags
    window.mouseJustPressed = 0
    window.mouseJustReleased = 0

    requestAnimationFrame(this.renderFrame)
  }

  async init(): Promise<void> {
    try {
      // Initialize CRT resources first
      await initCRT(this.canvas)
      this.resizeCanvas()

      // Load WASM module
      this.wasmModule = await createModule({
        canvas: this.canvas,
        print: (t: string) => console.log('[nh]', t),
        printErr: (t: string) => console.error('[nh]', t)
      })

      window.addEventListener('resize', () => this.resizeCanvas())

      // Wait for next frame then init game
      await new Promise((r) => requestAnimationFrame(r))
      this.wasmModule._game_init()
      this.lastTime = performance.now()
      requestAnimationFrame(this.renderFrame)
    } catch (e) {
      console.error('Init failed:', e)
    }
  }
}

// Wait for DOM and WASM to be ready
if (typeof createModule !== 'undefined') {
  const game = new Game()
  game.init()
} else {
  // Wait for game.js to load
  window.addEventListener('load', () => {
    if (typeof createModule !== 'undefined') {
      const game = new Game()
      game.init()
    }
  })
}

