import type { WasmModule } from './types'
import { audioManager, SoundType } from './audio'

// Extended key map for text editing
const keyMap: Record<string, number> = {
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

// Letter keys (a-z) start at 100
for (let i = 0; i < 26; i++) {
  keyMap['Key' + String.fromCharCode(65 + i)] = 100 + i
}

// Digit keys (0-9) start at 200
for (let i = 0; i < 10; i++) {
  keyMap['Digit' + i] = 200 + i
  keyMap['Numpad' + i] = 200 + i
}

// Special characters
const specialKeys: Record<string, number> = {
  Minus: 210,
  Equal: 211,
  BracketLeft: 212,
  BracketRight: 213,
  Semicolon: 214,
  Quote: 215,
  Backquote: 216,
  Backslash: 217,
  Comma: 218,
  Period: 219,
  Slash: 220
}
Object.assign(keyMap, specialKeys)

// Track shift state
let shiftHeld = false

// Value tagging for WASM interop
function tagInt(n: number): number {
  return (n << 1) | 1
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

    const key = keyMap[e.code]
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

    const key = keyMap[e.code]
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

