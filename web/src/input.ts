import type { WasmModule } from './types'
import { audioManager, SoundType } from './audio'

// Navigation/positional keys - use e.code (physical position)
// These are sent via the key code system
const codeKeyMap: Record<string, number> = {
  ArrowLeft: 0,
  ArrowRight: 1,
  ArrowUp: 2,
  ArrowDown: 3,
  Space: 4,
  Enter: 5,
  Tab: 10,
  Backspace: 11,
  Delete: 12,
  Home: 13,
  End: 14,
  Escape: 15
}

// Letter keys (a-z) key codes for key press detection (vim mode, etc.)
const letterKeyMap: Record<string, number> = {}
for (let i = 0; i < 26; i++) {
  const lower = String.fromCharCode(97 + i) // 'a' to 'z'
  const upper = String.fromCharCode(65 + i) // 'A' to 'Z'
  letterKeyMap[lower] = 100 + i
  letterKeyMap[upper] = 100 + i
}

// Digit keys (0-9) key codes
const digitKeyMap: Record<string, number> = {}
for (let i = 0; i < 10; i++) {
  digitKeyMap[String(i)] = 200 + i
}

// Track shift state
let shiftHeld = false

// Value tagging for WASM interop
function tagInt(n: number): number {
  return (n << 1) | 1
}

// Check if a character is printable (should be sent directly as char input)
function isPrintableChar(key: string): boolean {
  // Single character that is printable ASCII or common extended chars
  if (key.length !== 1) return false
  const code = key.charCodeAt(0)
  // Space (32) through tilde (126), plus common extended like £ (163)
  return code >= 32 && code <= 126 || code === 163
}

// Get key code for navigation/control keys
function getNavigationKeyCode(e: KeyboardEvent): number | undefined {
  return codeKeyMap[e.code]
}

// Get key code for letter keys (used for vim mode detection)
function getLetterKeyCode(e: KeyboardEvent): number | undefined {
  return letterKeyMap[e.key]
}

// Get key code for digit keys
function getDigitKeyCode(e: KeyboardEvent): number | undefined {
  return digitKeyMap[e.key]
}

export function setupKeyboardInput(getWasmModule: () => WasmModule | null): void {
  document.addEventListener('keydown', (e: KeyboardEvent) => {
    const wasmModule = getWasmModule()

    if (e.key === 'Shift') {
      shiftHeld = true
      wasmModule?._on_shift_down()
      return
    }

    // Handle Ctrl+C (copy)
    if ((e.ctrlKey || e.metaKey) && e.key === 'c') {
      window.clipboardCopyRequested = 1
      e.preventDefault()
      return
    }

    // Handle Ctrl+V (paste) - let browser handle it
    if ((e.ctrlKey || e.metaKey) && e.key === 'v') {
      return
    }

    // Handle Ctrl+A (select all)
    if ((e.ctrlKey || e.metaKey) && e.key === 'a') {
      window.selectAllRequested = 1
      e.preventDefault()
      return
    }

    // Skip if any modifier except shift is held (for shortcuts)
    if (e.ctrlKey || e.metaKey || e.altKey) {
      return
    }

    // Check for navigation keys first (arrows, enter, backspace, etc.)
    const navKey = getNavigationKeyCode(e)
    if (navKey !== undefined && wasmModule) {
      audioManager.play(SoundType.KEY_TYPE)
      wasmModule._on_key_down(tagInt(navKey))
      e.preventDefault()
      return
    }

    // Send letter key codes (for vim mode / game control detection)
    const letterKey = getLetterKeyCode(e)
    if (letterKey !== undefined && wasmModule) {
      wasmModule._on_key_down(tagInt(letterKey))
    }

    // Send digit key codes
    const digitKey = getDigitKeyCode(e)
    if (digitKey !== undefined && wasmModule) {
      wasmModule._on_key_down(tagInt(digitKey))
    }

    // For printable characters, also send the actual character code directly
    // This allows keyboard-layout-independent text input
    if (isPrintableChar(e.key) && wasmModule) {
      const charCode = e.key.charCodeAt(0)
      wasmModule._on_char_input(tagInt(charCode))
      audioManager.play(SoundType.KEY_TYPE)
      e.preventDefault()
    }
  })

  document.addEventListener('paste', (e: ClipboardEvent) => {
    const text = e.clipboardData?.getData('text')
    if (text) {
      window.clipboardText = text
      window.clipboardPasteRequested = 1
    }
    e.preventDefault()
  })

  document.addEventListener('keyup', (e: KeyboardEvent) => {
    const wasmModule = getWasmModule()

    if (e.key === 'Shift') {
      shiftHeld = false
      wasmModule?._on_shift_up()
      return
    }

    // Release navigation keys
    const navKey = getNavigationKeyCode(e)
    if (navKey !== undefined && wasmModule) {
      wasmModule._on_key_up(tagInt(navKey))
    }

    // Release letter keys
    const letterKey = getLetterKeyCode(e)
    if (letterKey !== undefined && wasmModule) {
      wasmModule._on_key_up(tagInt(letterKey))
    }

    // Release digit keys
    const digitKey = getDigitKeyCode(e)
    if (digitKey !== undefined && wasmModule) {
      wasmModule._on_key_up(tagInt(digitKey))
    }
  })
}

export function setupMouseInput(canvas: HTMLCanvasElement): void {
  function getMousePos(e: MouseEvent): { x: number; y: number } {
    return {
      x: Math.floor(e.clientX),
      y: Math.floor(e.clientY)
    }
  }

  canvas.addEventListener('mousedown', (e: MouseEvent) => {
    const pos = getMousePos(e)
    window.mouseX = pos.x
    window.mouseY = pos.y
    window.mouseDown = 1
    window.mouseJustPressed = 1
  })

  canvas.addEventListener('mouseup', () => {
    window.mouseDown = 0
    window.mouseJustReleased = 1
  })

  canvas.addEventListener('mousemove', (e: MouseEvent) => {
    const pos = getMousePos(e)
    window.mouseX = pos.x
    window.mouseY = pos.y
  })

  // Scroll wheel support - accumulate delta until consumed
  canvas.addEventListener('wheel', (e: WheelEvent) => {
    // Normalize deltaY: positive = scroll down, negative = scroll up
    // Convert to lines (roughly 3 lines per scroll notch)
    const lines = Math.sign(e.deltaY) * Math.ceil(Math.abs(e.deltaY) / 40)
    window.scrollDeltaY += lines
    e.preventDefault()
  }, { passive: false })
}

export function isShiftHeld(): boolean {
  return shiftHeld
}
