export enum SoundType {
    MOVE = 1,
    ATTACK = 2,
    HIT_RECEIVED = 3,
    DIE = 4,
    GOLD = 5,
    UI_CLICK = 6,
    UI_START = 7,
    UI_STOP = 8,
    KEY_TYPE = 9,
    LETTER_OPEN = 10,
    LETTER_SIGN = 11,
    GAME_OVER = 12,
    LEVEL_UP = 13,
    CRT_OPEN = 14,
    BOOT_BEEP = 15,
    POWER_BUTTON = 16
}

class AudioManager {
    private ctx: AudioContext | null = null;
    private masterGain: GainNode | null = null;
    private compressor: DynamicsCompressorNode | null = null;

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
        this.masterGain.gain.value = 0.2; // Reduced volume/Default volume

        // Add compressor to normalize volume (limit loud peaks)
        this.compressor = this.ctx.createDynamicsCompressor();
        this.compressor.threshold.value = -24;
        this.compressor.knee.value = 30;
        this.compressor.ratio.value = 12;
        this.compressor.attack.value = 0.003;
        this.compressor.release.value = 0.25;

        this.masterGain.connect(this.compressor);
        this.compressor.connect(this.ctx.destination);

        console.log('[Audio] Initialized');
    }

    public setVolume(level: number) {
        // level is 0-100, map to 0.0-0.2
        if (!this.masterGain) {
            this.init();
        }
        if (this.masterGain) {
            this.masterGain.gain.value = (level / 100) * 0.2;
        }
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

            case SoundType.LETTER_OPEN: // Paper rustle
                this.playPaperRustle(t);
                break;

            case SoundType.LETTER_SIGN: // Pen scratch
                this.playPenScratch(t);
                break;

            case SoundType.GAME_OVER: // Womp Womp
                this.playWompWomp(t);
                break;

            case SoundType.LEVEL_UP: // Sparkly Ascend
                this.playLevelUp(t);
                break;

            case SoundType.CRT_OPEN: // High pitch rising sine
                this.playCrtOpen(t);
                break;

            case SoundType.BOOT_BEEP: // PC Speaker Beep
                this.playBootBeep(t);
                break;

            case SoundType.POWER_BUTTON: // Mechanical Click
                this.playMouseClick(t);
                break;
        }
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

    private playFootstep(t: number) {
        if (!this.ctx || !this.masterGain) return;

        // Crispy footstep: short high-frequency noise bursts
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

    private playSwipe(t: number) {
        if (!this.ctx || !this.masterGain) return;

        // ORGANIC SWORD SWOOSH
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
        bodyGain.gain.linearRampToValueAtTime(1.0, t + 0.04);
        bodyGain.gain.setTargetAtTime(0, t + 0.04, duration / 2);

        bodySource.connect(bodyFilter);
        bodyFilter.connect(bodyGain);
        bodyGain.connect(this.masterGain);

        bodySource.start(t);
        bodySource.stop(t + duration + 0.2);
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

        // Highpass filter for crisp click
        const filter = this.ctx.createBiquadFilter();
        filter.type = 'highpass';
        filter.frequency.value = 800;

        noise.connect(filter);
        filter.connect(gain);
        gain.connect(this.masterGain);

        noise.start(t);
    }

    private playKeyType(t: number) {
        if (!this.ctx || !this.masterGain) return;

        // Keyboard clack: Muffled
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

    private playPaperRustle(t: number) {
        if (!this.ctx || !this.masterGain) return;

        // Paper rustle: Bandpassed noise with erratic volume modulation
        const duration = 0.5;
        const buffer = this.createNoiseBuffer(duration);
        const noise = this.ctx.createBufferSource();
        noise.buffer = buffer;

        // Bandpass filter centered around 1-2kHz
        const filter = this.ctx.createBiquadFilter();
        filter.type = 'bandpass';
        filter.frequency.value = 1200;
        filter.Q.value = 0.5;

        const gain = this.ctx.createGain();
        gain.gain.setValueAtTime(0, t);
        gain.gain.linearRampToValueAtTime(0.4, t + 0.1);
        gain.gain.exponentialRampToValueAtTime(0.01, t + duration);

        noise.connect(filter);
        filter.connect(gain);
        gain.connect(this.masterGain);

        noise.start(t);
        noise.stop(t + duration + 0.1);
    }

    private playPenScratch(t: number) {
        if (!this.ctx || !this.masterGain) return;

        // Pen scratch: Smoother granular synthesis
        // Less "ratchety" means we need softer attacks and higher friction frequencies
        const duration = 0.04 + Math.random() * 0.04; // 40-80ms grains
        const buffer = this.createNoiseBuffer(duration);
        const noise = this.ctx.createBufferSource();
        noise.buffer = buffer;

        // Bandpass at higher freq for smooth friction "shhh" rather than "scratch"
        const filter = this.ctx.createBiquadFilter();
        filter.type = 'bandpass';
        filter.frequency.value = 2000 + Math.random() * 1000; // 2000-3000Hz
        filter.Q.value = 0.8;

        const gain = this.ctx.createGain();
        // Quiet, consistent friction
        const vol = 0.1 + Math.random() * 0.05;

        gain.gain.setValueAtTime(0, t);
        // Soft bell-curve envelope to remove clicky attacks
        gain.gain.linearRampToValueAtTime(vol, t + duration * 0.5);
        gain.gain.linearRampToValueAtTime(0, t + duration);

        noise.connect(filter);
        filter.connect(gain);
        gain.connect(this.masterGain);

        noise.start(t);
        noise.stop(t + duration + 0.05);
    }

    private playWompWomp(t: number) {
        if (!this.ctx || !this.masterGain) return;

        // "Womp Womp" - descending triangle waves
        // Note 1: G3 (196Hz) -> C#3 (138Hz)
        // Note 2: F#3 (185Hz) -> C3 (130Hz)

        const playNote = (startFreq: number, endFreq: number, startTime: number, duration: number) => {
            if (!this.ctx || !this.masterGain) return;
            const osc = this.ctx.createOscillator();
            const gain = this.ctx.createGain();

            osc.type = 'triangle';
            osc.frequency.setValueAtTime(startFreq, startTime);
            osc.frequency.linearRampToValueAtTime(endFreq, startTime + duration);

            gain.gain.setValueAtTime(0, startTime);
            gain.gain.linearRampToValueAtTime(0.5, startTime + 0.05);
            gain.gain.setValueAtTime(0.5, startTime + duration - 0.05);
            gain.gain.linearRampToValueAtTime(0, startTime + duration);

            osc.connect(gain);
            gain.connect(this.masterGain);

            osc.start(startTime);
            osc.stop(startTime + duration + 0.1);
        };

        playNote(196, 138, t, 0.4);
        playNote(185, 130, t + 0.45, 0.6);
    }

    private playLevelUp(t: number) {
        if (!this.ctx || !this.masterGain) return;
        console.log('[Audio] Playing Level Up Sound');

        // Warmer ascending arpeggio (C4 Major)
        // Lower frequencies for a "power up" feel rather than high sparkle
        const notes = [
            261.63, // C4
            329.63, // E4
            392.00, // G4
            523.25, // C5
            659.25  // E5
        ];

        notes.forEach((freq, i) => {
            const osc = this.ctx!.createOscillator();
            const gain = this.ctx!.createGain();

            // Triangle wave for a bit more presence than sine, but still smooth
            osc.type = 'triangle';
            osc.frequency.value = freq;

            // Staggered starts
            const startTime = t + i * 0.08; // Slightly slower strum
            const duration = 0.6;

            gain.gain.setValueAtTime(0, startTime);
            gain.gain.linearRampToValueAtTime(0.15, startTime + 0.1);
            gain.gain.exponentialRampToValueAtTime(0.01, startTime + duration);

            osc.connect(gain);
            gain.connect(this.masterGain!);

            osc.start(startTime);
            osc.stop(startTime + duration + 0.1);
        });
    }

    private playCrtOpen(t: number) {
        if (!this.ctx || !this.masterGain) return;

        try {
            // CRT Open: High pitched sine wave sweeping up
            // Simulates the flyback transformer charging up
            const osc = this.ctx!.createOscillator();
            const gain = this.ctx!.createGain();

            osc.type = 'sine';
            osc.frequency.setValueAtTime(8000, t);
            osc.frequency.exponentialRampToValueAtTime(14000, t + 0.6);

            // Volume logic:
            gain.gain.setValueAtTime(0, t);
            gain.gain.linearRampToValueAtTime(0.05, t + 0.1);
            gain.gain.linearRampToValueAtTime(0.05, t + 0.5);
            gain.gain.exponentialRampToValueAtTime(0.001, t + 0.8);

            osc.connect(gain);
            gain.connect(this.masterGain);

            osc.start(t);
            osc.stop(t + 0.8);

            // Add a low hum to give it body (power surge)
            const hum = this.ctx!.createOscillator();
            const humGain = this.ctx!.createGain();
            hum.type = 'sawtooth';
            hum.frequency.setValueAtTime(60, t); // Main hum freq

            humGain.gain.setValueAtTime(0, t);
            humGain.gain.linearRampToValueAtTime(0.1, t + 0.05);
            humGain.gain.linearRampToValueAtTime(0, t + 0.4);

            const filter = this.ctx!.createBiquadFilter();
            filter.type = 'lowpass';
            filter.frequency.value = 200;

            hum.connect(filter);
            filter.connect(humGain);
            humGain.connect(this.masterGain);

            hum.start(t);
            hum.stop(t + 0.4);

            // Static crackles (static charge)
            const crackleDur = 0.5;
            const buffer = this.createNoiseBuffer(crackleDur);
            const noise = this.ctx.createBufferSource();
            noise.buffer = buffer;

            const noiseFilter = this.ctx!.createBiquadFilter();
            noiseFilter.type = 'highpass';
            noiseFilter.frequency.value = 3000;

            const noiseGain = this.ctx!.createGain();
            noiseGain.gain.setValueAtTime(0, t);
            noiseGain.gain.linearRampToValueAtTime(0.2, t + 0.05);
            noiseGain.gain.exponentialRampToValueAtTime(0.01, t + 0.3);

            noise.connect(noiseFilter);
            noiseFilter.connect(noiseGain);
            noiseGain.connect(this.masterGain);

            noise.start(t);
        } catch (e) {
            console.error('[Audio] CRT Open Error:', e);
        }
    }

    private playBootBeep(t: number) {
        if (!this.ctx || !this.masterGain) return;

        // Classic PC Boot Beep
        // Square wave, ~800Hz-1000Hz, short duration
        const osc = this.ctx!.createOscillator();
        const gain = this.ctx!.createGain();

        osc.type = 'square';
        osc.frequency.setValueAtTime(880, t); // A5

        gain.gain.setValueAtTime(0.1, t);
        gain.gain.setValueAtTime(0.1, t + 0.1);
        gain.gain.linearRampToValueAtTime(0, t + 0.15);

        osc.connect(gain);
        gain.connect(this.masterGain);

        osc.start(t);
        osc.stop(t + 0.2);
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
}

// Export singleton
export const audioManager = new AudioManager();
