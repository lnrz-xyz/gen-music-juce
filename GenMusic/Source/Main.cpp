#include <JuceHeader.h>
#include <iostream>
#include <sstream>

template<typename T>
void log(const T& value) {
    std::cout << value << std::endl;
}

template<typename T, typename... Args>
void log(const T& first, const Args&... args) {
    std::cout << first << " ";
    log(args...);
}


class DefaultSynthSound : public juce::SynthesiserSound {
public:
    bool appliesToNote (int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel (int /*midiChannel*/) override { return true; }
};


class SampleVoice : public juce::SynthesiserVoice {
public:
    SampleVoice(const std::string& samplePath, int midiNoteForSample)
    : sampleRate(0), rootMidiNote(midiNoteForSample), isPlaying(false)
    {
        juce::File file(samplePath);
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        
        if (reader.get() != nullptr) {
            sampleRate = reader->sampleRate;
            sample.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
            reader->read(&sample, 0, (int)reader->lengthInSamples, 0, true, true);
            log(sampleRate, reader->numChannels, reader->lengthInSamples, "reader details");
        }
    }
    
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        auto result = dynamic_cast<DefaultSynthSound*>(sound) != nullptr;
        log(result, "can play");
        return result;
    }
    
    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound* sound, int currentPitchWheelPosition) override {
        // Calculate pitch ratio based on root note and incoming note
        double frequencyOfNote = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        double frequencyOfRootNote = juce::MidiMessage::getMidiNoteInHertz(rootMidiNote);
        pitchRatio = frequencyOfNote / frequencyOfRootNote;
        
        // Set the position and other parameters as needed
        currentPosition = 0;
        log("starting note");
        isPlaying = true;
    }
    
    void stopNote(float velocity, bool allowTailOff) override {
        log("stopping note");
        isPlaying = false;
    }
    
    
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
        if (isPlaying) {
            log("playing");
            // Fill the output buffer with your sample data
            for (int sampleIdx = startSample; sampleIdx < startSample + numSamples; ++sampleIdx) {
                if (currentPosition < sample.getNumSamples()) {
                    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel) {
                        outputBuffer.addSample(channel, sampleIdx, sample.getSample(channel, static_cast<int>(currentPosition)) * pitchRatio);
                    }
                    currentPosition += pitchRatio;
                } else {
                    stopNote(0.0f, false);
                    break;
                }
            }
        } else {
            log("not playing");
        }
    }
    
    void pitchWheelMoved(int newValue) override {
        // Handle pitch wheel changes if needed
    }
    
    void controllerMoved(int controllerNumber, int newValue) override {
        // Handle other MIDI controllers if needed
    }
    
private:
    juce::AudioSampleBuffer sample;
    double sampleRate;
    int rootMidiNote;
    double currentPosition;
    double pitchRatio;
    bool isPlaying;
};




int main() {
    
    // Create a Synthesiser and add your custom voice
    juce::Synthesiser synth;
    synth.setCurrentPlaybackSampleRate(44100.0 ); // Set to the desired sample rate, e.g., 44100 Hz
    synth.setNoteStealingEnabled(true);
    for (int i = 0; i < 4; ++i) {
        synth.addVoice(new SampleVoice("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/JPIANO-C2.mp3", 48));
    }
    synth.addSound(new DefaultSynthSound());
    
    const double bpm = 120.0;
    const double sampleRate = 44100.0;
    const double quarterNoteLengthInSamples = (60.0 / bpm) * sampleRate;
    
    // Create a MIDI sequence
    juce::MidiMessageSequence mySequence;
    mySequence.addEvent(juce::MidiMessage::noteOn(1, 48, 0.8f), 0);
    mySequence.addEvent(juce::MidiMessage::noteOff(1, 48), quarterNoteLengthInSamples*3 );
    mySequence.addEvent(juce::MidiMessage::noteOn(1, 52, 0.8f), quarterNoteLengthInSamples * 2);
    mySequence.addEvent(juce::MidiMessage::noteOff(1, 52), quarterNoteLengthInSamples * 10);
    mySequence.updateMatchedPairs();
    // Repeat for the other notes
    
    juce::MidiBuffer midiBuffer;

    for (int i = 0; i < mySequence.getNumEvents(); ++i) {
        auto* midiEvent = mySequence.getEventPointer(i);
        int sampleNumber = (int)(midiEvent->message.getTimeStamp()); // Assuming 44100 Hz sample rate
        log(sampleNumber, "sample number");
        midiBuffer.addEvent(midiEvent->message, sampleNumber);
    }
    
    
    // Set up an AudioBuffer to render to
    juce::AudioBuffer<float> buffer;
    buffer.clear();
    buffer.setSize(2, 44100 * 10); // Stereo buffer, 6 seconds long
    log(buffer.getNumSamples(), buffer.getNumChannels(), mySequence.getEndTime(), "samples/channels");
    
    // Convert the sequence to a MidiBuffer...
    synth.renderNextBlock(buffer, midiBuffer, 0, buffer.getNumSamples());
    
    // Write to a WAV file
    juce::File outputFile("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/output.wav");
    if (outputFile.exists()) {
        outputFile.deleteFile();
    }
    auto* outputStream = new juce::FileOutputStream(outputFile);
    
    if (outputStream->failedToOpen())
    {
        delete outputStream;
        return 1;
    }
    
    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer(wavFormat.createWriterFor(outputStream, 44100, buffer.getNumChannels(), 16, {}, 0));
    if (writer != nullptr) {
        writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
    }
    else {
        delete outputStream;
    }
    
    
    return 0;
}
