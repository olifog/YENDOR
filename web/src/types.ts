// Type definitions for the WASM module and global window extensions

export interface WasmModule {
  _game_init(): void
  _game_update(dt: number): void
  _game_render(): void
  _on_frame_start(): void
  _on_key_down(key: number): void
  _on_key_up(key: number): void
  _on_shift_down(): void
  _on_shift_up(): void
}

declare global {
  interface Window {
    textCtx: CanvasRenderingContext2D | null
    screenWidth: number
    screenHeight: number
    mouseX: number
    mouseY: number
    mouseDown: number
    mouseJustPressed: number
    mouseJustReleased: number
    scrollDeltaY: number
    clipboardText: string
    clipboardPasteRequested: number
    clipboardCopyRequested: number
    selectAllRequested: number
    skipCRT: number
  }

  function createModule(options: {
    canvas: HTMLCanvasElement
    print: (text: string) => void
    printErr: (text: string) => void
  }): Promise<WasmModule>
}

export {}

