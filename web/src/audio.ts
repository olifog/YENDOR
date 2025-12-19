
export enum SoundType {
    MOVE = 1,
    ATTACK = 2,
    HIT_RECEIVED = 3,
    DIE = 4,
    GOLD = 5,
    UI_CLICK = 6,
    UI_START = 7,
    UI_STOP = 8,
    KEY_TYPE = 9
}

class AudioManager {
    private ctx: AudioContext | null = null;
    private masterGain: GainNode | null = null;

    constructor() {
        // We defer initialization until the first interaction to satisfy browser policies
        window.addEventListener('click', () => this.init(), { once: true });
        window.addEventListener('keydown', () => this.init(), { once: true });
    }

    private init() {
        if (this.ctx) return;
        const AudioContext = window.AudioContext || (window as any).webkitAudioContext;
        if (!AudioContext) return;

        this.ctx = new AudioContext();
        this.masterGain = this.ctx.createGain();
        this.masterGain.gain.value = 0.3; // Default volume
        this.masterGain.connect(this.ctx.destination);

        console.log('[Audio] Initialized');
    }

    public play(id: number) {
        if (!this.ctx) this.init();
        if (!this.ctx || this.ctx.state === 'suspended') {
            this.ctx?.resume();
        }

        if (!this.ctx || !this.masterGain) return;

        const t = this.ctx.currentTime;

        switch (id) {
            case SoundType.GOLD: // Classic coin pickup
                this.playCoinPickup(t);
                break;

            case SoundType.MOVE: // Footstep
                this.playFootstep(t);
                break;

            case SoundType.ATTACK: // Swipe
                this.playSwipe(t);
                break;

            case SoundType.HIT_RECEIVED: // Blunt Thud
                this.playThud(t);
                break;

            case SoundType.DIE: // Descending
                this.playDescending(t);
                break;

            case SoundType.UI_CLICK: // Mouse click
                this.playMouseClick(t);
                break;

            case SoundType.UI_START: // Ascending
                this.playMouseClick(t);
                break;

            case SoundType.UI_STOP: // Descending
                this.playMouseClick(t);
                break;

            case SoundType.KEY_TYPE: // Keyboard clack
                this.playKeyType(t);
                break;
        }
    }

    private playThud(t: number) {
        if (!this.ctx || !this.masterGain) return;

        // Blunt thud: Low frequency sine/triangle with fast pitch drop
        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();

        osc.type = 'triangle';
        osc.frequency.setValueAtTime(150, t);
        osc.frequency.exponentialRampToValueAtTime(40, t + 0.15);

        gain.gain.setValueAtTime(0.8, t);
        gain.gain.exponentialRampToValueAtTime(0.01, t + 0.15);

        // Lowpass filter to make it "dull" and blunt
        const filter = this.ctx.createBiquadFilter();
        filter.type = 'lowpass';
        filter.frequency.value = 300; // Very muffled

        osc.connect(filter);
        filter.connect(gain);
        gain.connect(this.masterGain);

        osc.start(t);
        osc.stop(t + 0.2);
    }

    private playKeyType(t: number) {
        if (!this.ctx || !this.masterGain) return;

        // Keyboard clack: Similar to mouse click but lower pitched
        // Using bandpass around 300-400Hz for that "plastic switch" sound
        const bufferSize = Math.floor(this.ctx.sampleRate * 0.025); // 25ms
        const buffer = this.ctx.createBuffer(1, bufferSize, this.ctx.sampleRate);
        const data = buffer.getChannelData(0);

        for (let i = 0; i < bufferSize; i++) {
            const envelope = Math.exp(-i / (bufferSize * 0.15));
            data[i] = (Math.random() * 2 - 1) * envelope;
        }

        const noise = this.ctx.createBufferSource();
        noise.buffer = buffer;

        const gain = this.ctx.createGain();
        gain.gain.value = 1.6;

        const filter = this.ctx.createBiquadFilter();
        filter.type = 'lowpass'; // Muffled sound
        filter.frequency.value = 600; // Cut off high frequencies for "thock" sound
        filter.Q.value = 0.5;

        noise.connect(filter);
        filter.connect(gain);
        gain.connect(this.masterGain);

        noise.start(t);
    }

    private playTone(freq: number, type: OscillatorType, attack: number, decay: number) {
        if (!this.ctx || !this.masterGain) return;

        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();

        osc.type = type;
        osc.frequency.setValueAtTime(freq, this.ctx.currentTime);

        gain.connect(this.masterGain);
        osc.connect(gain);

        const t = this.ctx.currentTime;
        gain.gain.setValueAtTime(0, t);
        gain.gain.linearRampToValueAtTime(0.5, t + attack);
        gain.gain.exponentialRampToValueAtTime(0.01, t + attack + decay);

        osc.start(t);
        osc.stop(t + attack + decay + 0.1);
    }

    private playNoise(duration: number, volume: number) {
        if (!this.ctx || !this.masterGain) return;

        const bufferSize = this.ctx.sampleRate * duration;
        const buffer = this.ctx.createBuffer(1, bufferSize, this.ctx.sampleRate);
        const data = buffer.getChannelData(0);

        for (let i = 0; i < bufferSize; i++) {
            data[i] = Math.random() * 2 - 1;
        }

        const noise = this.ctx.createBufferSource();
        noise.buffer = buffer;

        const gain = this.ctx.createGain();
        gain.gain.value = volume;

        // Lowpass for "thud"
        const filter = this.ctx.createBiquadFilter();
        filter.type = 'lowpass';
        filter.frequency.value = 200;

        noise.connect(filter);
        filter.connect(gain);
        gain.connect(this.masterGain);

        noise.start();
    }

    private playSwipe(t: number) {
        if (!this.ctx || !this.masterGain) return;

        // ORGANIC SWORD SWOOSH
        // Layer 1: The "Air" (High frequency hiss)
        // Layer 2: The "Body" (Wind displacement)

        const variance = Math.random() * 0.1 - 0.05; // +/- 50ms random var
        const duration = 0.18 + variance;

        // --- Layer 1: High Frequency Air ---
        const airBuffer = this.createNoiseBuffer(duration);
        const airSource = this.ctx.createBufferSource();
        airSource.buffer = airBuffer;

        const airFilter = this.ctx.createBiquadFilter();
        airFilter.type = 'highpass';
        airFilter.frequency.setValueAtTime(4000 + (Math.random() * 1000), t); // Random start
        airFilter.frequency.exponentialRampToValueAtTime(100, t + duration); // Sweep down

        const airGain = this.ctx.createGain();
        airGain.gain.setValueAtTime(0, t);
        airGain.gain.linearRampToValueAtTime(0.8, t + 0.02);
        // Organic decay
        airGain.gain.setTargetAtTime(0, t + 0.02, duration / 3);

        airSource.connect(airFilter);
        airFilter.connect(airGain);
        airGain.connect(this.masterGain);

        airSource.start(t);
        airSource.stop(t + duration + 0.2);

        // --- Layer 2: Mid/Low Body (Wind) ---
        const bodyBuffer = this.createNoiseBuffer(duration);
        const bodySource = this.ctx.createBufferSource();
        bodySource.buffer = bodyBuffer;

        const bodyFilter = this.ctx.createBiquadFilter();
        bodyFilter.type = 'lowpass';
        bodyFilter.frequency.setValueAtTime(600, t);
        bodyFilter.frequency.linearRampToValueAtTime(100, t + duration);

        const bodyGain = this.ctx.createGain();
        bodyGain.gain.setValueAtTime(0, t);
        bodyGain.gain.linearRampToValueAtTime(1.0, t + 0.04); // Slightly slower attack than air
        bodyGain.gain.setTargetAtTime(0, t + 0.04, duration / 2);

        bodySource.connect(bodyFilter);
        bodyFilter.connect(bodyGain);
        bodyGain.connect(this.masterGain);

        //bodySource.start(t);
        //bodySource.stop(t + duration + 0.2);
    }

    // Helper to generate fresh noise (prevents "frozen" noise patterns)
    private createNoiseBuffer(duration: number): AudioBuffer {
        const size = Math.floor(this.ctx!.sampleRate * duration);
        const buffer = this.ctx!.createBuffer(1, size, this.ctx!.sampleRate);
        const data = buffer.getChannelData(0);
        for (let i = 0; i < size; i++) {
            data[i] = Math.random() * 2 - 1;
        }
        return buffer;
    }

    private playDescending(t: number) {
        if (!this.ctx || !this.masterGain) return;

        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();

        osc.type = 'sawtooth';
        osc.frequency.setValueAtTime(400, t);
        osc.frequency.linearRampToValueAtTime(50, t + 1.0);

        gain.gain.setValueAtTime(0.3, t);
        gain.gain.linearRampToValueAtTime(0, t + 1.0);

        osc.connect(gain);
        gain.connect(this.masterGain);

        osc.start(t);
        osc.stop(t + 1.0);
    }

    private playCoinPickup(t: number) {
        if (!this.ctx || !this.masterGain) return;

        // Classic coin sound: quick ascending arpeggio with a high sparkle
        // Similar to Mario coin or Zelda rupee
        const notes = [987.77, 1318.51]; // B5, E6 - the classic coin interval
        const duration = 0.08;

        notes.forEach((freq, i) => {
            const osc = this.ctx!.createOscillator();
            const gain = this.ctx!.createGain();

            osc.type = 'square';
            osc.frequency.setValueAtTime(freq, t + i * duration);

            gain.gain.setValueAtTime(0, t + i * duration);
            gain.gain.linearRampToValueAtTime(0.3, t + i * duration + 0.01);
            gain.gain.exponentialRampToValueAtTime(0.01, t + i * duration + duration * 1.5);

            osc.connect(gain);
            gain.connect(this.masterGain!);

            osc.start(t + i * duration);
            osc.stop(t + i * duration + duration * 2);
        });
    }

    private playMouseClick(t: number) {
        if (!this.ctx || !this.masterGain) return;

        // Mouse click: very short, sharp noise burst with high-frequency content
        const bufferSize = Math.floor(this.ctx.sampleRate * 0.02); // 20ms
        const buffer = this.ctx.createBuffer(1, bufferSize, this.ctx.sampleRate);
        const data = buffer.getChannelData(0);

        // Create a sharp click envelope
        for (let i = 0; i < bufferSize; i++) {
            const envelope = Math.exp(-i / (bufferSize * 0.08)); // Very fast decay
            data[i] = (Math.random() * 2 - 1) * envelope;
        }

        const noise = this.ctx.createBufferSource();
        noise.buffer = buffer;

        const gain = this.ctx.createGain();
        gain.gain.value = 1.5;

        // Highpass filter for crisp click (less energy loss than bandpass)
        const filter = this.ctx.createBiquadFilter();
        filter.type = 'highpass';
        filter.frequency.value = 800;

        noise.connect(filter);
        filter.connect(gain);
        gain.connect(this.masterGain);

        noise.start(t);
    }

    private playFootstep(t: number) {
        if (!this.ctx || !this.masterGain) return;

        // Crispy footstep: short high-frequency noise burst
        const bufferSize = Math.floor(this.ctx.sampleRate * 0.03); // 30ms
        const buffer = this.ctx.createBuffer(1, bufferSize, this.ctx.sampleRate);
        const data = buffer.getChannelData(0);

        // Sharp attack with quick decay
        for (let i = 0; i < bufferSize; i++) {
            const envelope = Math.exp(-i / (bufferSize * 0.15));
            data[i] = (Math.random() * 2 - 1) * envelope;
        }

        const noise = this.ctx.createBufferSource();
        noise.buffer = buffer;

        const gain = this.ctx.createGain();
        gain.gain.value = 0.4;

        // Highpass for crispy step sound
        const filter = this.ctx.createBiquadFilter();
        filter.type = 'highpass';
        filter.frequency.value = 1500;

        noise.connect(filter);
        filter.connect(gain);
        gain.connect(this.masterGain);

        noise.start(t);
    }
}

// Export singleton
export const audioManager = new AudioManager();
