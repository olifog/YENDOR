// CRT Shader WebGL Renderer

let gl: WebGL2RenderingContext | null = null
let crtProgram: WebGLProgram | null = null
let passthroughProgram: WebGLProgram | null = null
let quadVAO: WebGLVertexArrayObject | null = null

// Framebuffers & Textures
let sceneFBO: WebGLFramebuffer | null = null
let sceneTexture: WebGLTexture | null = null
let textTexture: WebGLTexture | null = null

export function getGL(): WebGL2RenderingContext | null {
  return gl
}

export function getSceneFBO(): WebGLFramebuffer | null {
  return sceneFBO
}

function createShader(
  gl: WebGL2RenderingContext,
  type: number,
  source: string
): WebGLShader | null {
  const shader = gl.createShader(type)
  if (!shader) return null

  gl.shaderSource(shader, source)
  gl.compileShader(shader)

  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    console.error('Shader compile error:', gl.getShaderInfoLog(shader))
    gl.deleteShader(shader)
    return null
  }

  return shader
}

export async function initCRT(canvas: HTMLCanvasElement): Promise<void> {
  gl = canvas.getContext('webgl2')
  if (!gl) {
    throw new Error('WebGL 2 not supported')
  }

  // Fix inverted text texture
  gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true)

  // Fetch Shader Code
  const response = await fetch('/shaders/crt.glsl')
  const crtFunctions = await response.text()

  const vsSource = `#version 300 es
    layout(location = 0) in vec2 aPos;
    out vec2 vUv;
    void main() {
      vUv = aPos * 0.5 + 0.5;
      gl_Position = vec4(aPos, 0.0, 1.0);
    }`

  const fsSource = `#version 300 es
    precision highp float;
    out vec4 FragColor;
    in vec2 vUv;
    
    uniform sampler2D uScene;
    uniform sampler2D uText;
    uniform vec2 iResolution;
    uniform float iTime;

    ${crtFunctions}

    void main() {
      // Apply CRT distortion to UVs
      vec2 crtUv = crtCurve(vUv);
      
      // If out of bounds, return black
      if (crtUv.x < 0.0 || crtUv.x > 1.0 || crtUv.y < 0.0 || crtUv.y > 1.0) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
      }

      // Sample Scene and Text at distorted UVs
      vec4 sceneCol = texture(uScene, crtUv);
      vec4 textCol = texture(uText, crtUv);
      vec3 textRGB = sampleChromatic(uText, crtUv);

      vec3 finalCol = mix(sceneCol.rgb, textRGB, textCol.a);

      // Apply CRT effects (scanlines, vignette, etc.)
      finalCol = applyCRT(crtUv * iResolution, finalCol);

      FragColor = vec4(finalCol, 1.0);
    }`

  // Compile Shaders
  const vs = createShader(gl, gl.VERTEX_SHADER, vsSource)
  const fs = createShader(gl, gl.FRAGMENT_SHADER, fsSource)

  if (!vs || !fs) {
    throw new Error('Failed to compile shaders')
  }

  crtProgram = gl.createProgram()
  if (!crtProgram) {
    throw new Error('Failed to create program')
  }

  gl.attachShader(crtProgram, vs)
  gl.attachShader(crtProgram, fs)
  gl.linkProgram(crtProgram)

  if (!gl.getProgramParameter(crtProgram, gl.LINK_STATUS)) {
    throw new Error('Program link error: ' + gl.getProgramInfoLog(crtProgram))
  }

  // Create passthrough shader (for intro - no CRT effects)
  const passthroughFsSource = `#version 300 es
    precision highp float;
    out vec4 FragColor;
    in vec2 vUv;
    uniform sampler2D uText;
    
    void main() {
      vec4 textCol = texture(uText, vUv);
      FragColor = textCol;
    }`

  const passthroughVs = createShader(gl, gl.VERTEX_SHADER, vsSource)
  const passthroughFs = createShader(gl, gl.FRAGMENT_SHADER, passthroughFsSource)

  if (passthroughVs && passthroughFs) {
    passthroughProgram = gl.createProgram()
    if (passthroughProgram) {
      gl.attachShader(passthroughProgram, passthroughVs)
      gl.attachShader(passthroughProgram, passthroughFs)
      gl.linkProgram(passthroughProgram)
    }
  }

  // Fullscreen Quad
  const vertices = new Float32Array([
    -1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1
  ])

  quadVAO = gl.createVertexArray()
  const quadVBO = gl.createBuffer()

  gl.bindVertexArray(quadVAO)
  gl.bindBuffer(gl.ARRAY_BUFFER, quadVBO)
  gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW)
  gl.enableVertexAttribArray(0)
  gl.vertexAttribPointer(0, 2, gl.FLOAT, false, 0, 0)
  gl.bindVertexArray(null)

  // Initialize Framebuffers
  initFramebuffers(canvas.width, canvas.height)
}

export function initFramebuffers(width: number, height: number): void {
  if (!gl) return

  // 1. Scene FBO (Game renders here)
  if (sceneTexture) gl.deleteTexture(sceneTexture)
  if (sceneFBO) gl.deleteFramebuffer(sceneFBO)

  sceneTexture = gl.createTexture()
  gl.bindTexture(gl.TEXTURE_2D, sceneTexture)
  gl.texImage2D(
    gl.TEXTURE_2D,
    0,
    gl.RGB,
    width,
    height,
    0,
    gl.RGB,
    gl.UNSIGNED_BYTE,
    null
  )
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR)
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)

  sceneFBO = gl.createFramebuffer()
  gl.bindFramebuffer(gl.FRAMEBUFFER, sceneFBO)
  gl.framebufferTexture2D(
    gl.FRAMEBUFFER,
    gl.COLOR_ATTACHMENT0,
    gl.TEXTURE_2D,
    sceneTexture,
    0
  )

  // 2. Text Texture (We upload canvas here)
  if (textTexture) gl.deleteTexture(textTexture)
  textTexture = gl.createTexture()
  gl.bindTexture(gl.TEXTURE_2D, textTexture)
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR)
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)

  gl.bindFramebuffer(gl.FRAMEBUFFER, null)
}

export function renderCRTPass(
  canvas: HTMLCanvasElement,
  textCanvas: HTMLCanvasElement,
  time: number
): void {
  if (!gl || !crtProgram || !quadVAO) return

  // Upload Text Canvas to Texture
  gl.bindTexture(gl.TEXTURE_2D, textTexture)
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, textCanvas)

  // Composite to Screen using CRT Shader
  gl.bindFramebuffer(gl.FRAMEBUFFER, null)
  gl.viewport(0, 0, canvas.width, canvas.height)
  gl.clear(gl.COLOR_BUFFER_BIT)

  gl.useProgram(crtProgram)

  // Set Uniforms
  gl.uniform1i(gl.getUniformLocation(crtProgram, 'uScene'), 0)
  gl.uniform1i(gl.getUniformLocation(crtProgram, 'uText'), 1)
  gl.uniform2f(
    gl.getUniformLocation(crtProgram, 'iResolution'),
    canvas.width,
    canvas.height
  )
  gl.uniform1f(gl.getUniformLocation(crtProgram, 'iTime'), time / 1000.0)

  // Bind Textures
  gl.activeTexture(gl.TEXTURE0)
  gl.bindTexture(gl.TEXTURE_2D, sceneTexture)
  gl.activeTexture(gl.TEXTURE1)
  gl.bindTexture(gl.TEXTURE_2D, textTexture)

  // Draw Quad
  gl.bindVertexArray(quadVAO)
  gl.drawArrays(gl.TRIANGLES, 0, 6)
  gl.bindVertexArray(null)
}

// Render without CRT effects (for intro/menus)
export function renderPassthrough(
  canvas: HTMLCanvasElement,
  textCanvas: HTMLCanvasElement
): void {
  if (!gl || !passthroughProgram || !quadVAO) return

  // Upload Text Canvas to Texture
  gl.bindTexture(gl.TEXTURE_2D, textTexture)
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, textCanvas)

  // Render directly to screen
  gl.bindFramebuffer(gl.FRAMEBUFFER, null)
  gl.viewport(0, 0, canvas.width, canvas.height)
  gl.clearColor(0.96, 0.92, 0.86, 1) // Cream background
  gl.clear(gl.COLOR_BUFFER_BIT)

  gl.useProgram(passthroughProgram)

  // Set Uniforms
  gl.uniform1i(gl.getUniformLocation(passthroughProgram, 'uText'), 0)

  // Bind Texture
  gl.activeTexture(gl.TEXTURE0)
  gl.bindTexture(gl.TEXTURE_2D, textTexture)

  // Draw Quad
  gl.bindVertexArray(quadVAO)
  gl.drawArrays(gl.TRIANGLES, 0, 6)
  gl.bindVertexArray(null)
}

