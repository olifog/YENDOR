import type { WasmModule } from './types'
import { audioManager, SoundType } from './audio'

// Navigation/positional keys - use e.code (physical position)
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

// Character keys - use e.key (actual character typed) for keyboard layout independence
// This allows AZERTY, QWERTZ, Dvorak, and other layouts to work correctly
const keyKeyMap: Record<string, number> = {}

// Letter keys (a-z) start at 100 - map by actual character, not physical key
for (let i = 0; i < 26; i++) {
  const lower = String.fromCharCode(97 + i) // 'a' to 'z'
  const upper = String.fromCharCode(65 + i) // 'A' to 'Z'
  keyKeyMap[lower] = 100 + i
  keyKeyMap[upper] = 100 + i
}

// Digit keys (0-9) start at 200
for (let i = 0; i < 10; i++) {
  keyKeyMap[String(i)] = 200 + i
}

// Special characters - map by actual character for layout independence
const specialKeyMap: Record<string, number> = {
  '-': 210,
  '_': 210,
  '=': 211,
  '+': 211,
  '[': 212,
  '{': 212,
  ']': 213,
  '}': 213,
  ';': 214,
  ':': 214,
  "'": 215,
  '"': 215,
  '`': 216,
  '~': 216,
  '\\': 217,
  '|': 217,
  ',': 218,
  '<': 218,
  '.': 219,
  '>': 219,
  '/': 220,
  '?': 220
}
Object.assign(keyKeyMap, specialKeyMap)

// Track shift state
let shiftHeld = false

// Value tagging for WASM interop
function tagInt(n: number): number {
  return (n << 1) | 1
}

// Get key code from event, supporting both layout-independent and positional keys
function getKeyCode(e: KeyboardEvent): number | undefined {
  // First check positional keys (arrows, enter, etc.) by physical key code
  const codeKey = codeKeyMap[e.code]
  if (codeKey !== undefined) {
    return codeKey
  }

  // Then check character keys by the actual character typed (layout-independent)
  const keyKey = keyKeyMap[e.key]
  if (keyKey !== undefined) {
    return keyKey
  }

  return undefined
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

    const key = getKeyCode(e)
    if (key !== undefined && wasmModule) {
      audioManager.play(SoundType.KEY_TYPE)
      wasmModule._on_key_down(tagInt(key))
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

    const key = getKeyCode(e)
    if (key !== undefined && wasmModule) {
      wasmModule._on_key_up(tagInt(key))
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

