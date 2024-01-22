#include <JuceHeader.h>

class MySynthSound : public juce::SynthesiserSound {
public:
    bool appliesToNote (int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel (int /*midiChannel*/) override { return true; }
};

class MySynthVoice : public juce::SynthesiserVoice {
private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioSourcePlayer audioSourcePlayer;
    juce::AudioTransportSource transportSource;
    bool isNotePlaying = false;
    double currentSampleRate;
    juce::LagrangeInterpolator interpolator;
    double sourceSampleRate;
    double pitchRatio = 1.0;
    
    
public:
    MySynthVoice() {
        formatManager.registerBasicFormats();
    }
    ~MySynthVoice() override {
        
    }
    
    bool canPlaySound (juce::SynthesiserSound* sound) override {
        return dynamic_cast<MySynthSound*>(sound) != nullptr;
    }
    
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* /*sound*/, int /*currentPitchWheelPosition*/) override {
        if (midiNoteNumber >= 0 && midiNoteNumber < 128) {
            juce::File audioFile { "/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/JPIANO-C2.mp3" };
            std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(audioFile));
            
            if (reader.get() != nullptr) {
                readerSource.reset(new juce::AudioFormatReaderSource(reader.release(), true));
                transportSource.setSource(readerSource.get());
                transportSource.start();
                isNotePlaying = true;
            }
            
            auto frequencyOfNote = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
            auto frequencyOfSample = juce::MidiMessage::getMidiNoteInHertz(48); // C2 is MIDI note 48
            pitchRatio = frequencyOfNote / frequencyOfSample;
            
            // Reset the interpolator and prepare for new playback
            
            interpolator.reset();
            
            std::cout << frequencyOfNote << " " << frequencyOfSample << " " << pitchRatio << " ratio\n";
            
        }
    }
    
    void stopNote (float /*velocity*/, bool allowTailOff) override {
        if (allowTailOff && isNotePlaying) {
            // Implement tail-off logic here if needed
        } else {
            // Stop the note immediately
            transportSource.stop();
            isNotePlaying = false;
        }
    }
    
    
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
        if (isNotePlaying && transportSource.isPlaying()) {
            juce::AudioBuffer<float> tempBuffer(outputBuffer.getNumChannels(), numSamples);
            juce::AudioSourceChannelInfo info(&tempBuffer, 0, numSamples);
            transportSource.getNextAudioBlock(info);
            
            for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel) {
                interpolator.process(pitchRatio, tempBuffer.getReadPointer(channel),
                                     outputBuffer.getWritePointer(channel, startSample), numSamples);
            }
        }
    }
    
    bool isVoiceActive() const override {
        return isNotePlaying; // Return the actual playing state
    }
    
    void aftertouchChanged (int newAftertouchValue) override {
        // Implement if needed
    }
    
    void channelPressureChanged (int newChannelPressureValue) override {
        // Implement if needed
    }
    
    void setCurrentPlaybackSampleRate (double newRate) override {
        
    }
    
    
    bool isPlayingChannel (int midiChannel) const override {
        // Return true if this voice is playing on the specified channel.
        // Update this to reflect the actual state of your voice.
        return true;
    }
    
    void pitchWheelMoved (int newPitchWheelValue) override {
        // Implement pitch wheel functionality if needed.
    }
    
    void controllerMoved (int controllerNumber, int newControllerValue) override {
        // Implement MIDI controller functionality if needed.
    }
};




int main() {
    // Load the piano sample
    // ...
    
    // Create a Synthesiser and add your custom voice
    juce::Synthesiser mySynth;
    mySynth.setCurrentPlaybackSampleRate(44100.0); // Set to the desired sample rate, e.g., 44100 Hz
    mySynth.addVoice(new MySynthVoice());
    mySynth.addSound(new MySynthSound());
    
    // Create a MIDI sequence
    juce::MidiMessageSequence mySequence;
    // Assuming MIDI note 60 (Middle C), duration 1 quarter note, starting at beat 0
    mySequence.addEvent(juce::MidiMessage::noteOn(1, 60, 0.8f), 0);
    mySequence.addEvent(juce::MidiMessage::noteOff(1, 60), 480); // 480 ticks for a quarter note
    
    // Repeat for the other notes
    
    std::cout << mySequence.getNumEvents() << "num events\n";
    
    juce::MidiBuffer midiBuffer;
    for (int i = 0; i < mySequence.getNumEvents(); ++i) {
        auto* midiEvent = mySequence.getEventPointer(i);
        int sampleNumber = (int)(midiEvent->message.getTimeStamp() * 44100.0); // Assuming 44100 Hz sample rate
        midiBuffer.addEvent(midiEvent->message, sampleNumber);
    }
    
    // Set up an AudioBuffer to render to
    juce::AudioBuffer<float> buffer;
    buffer.clear();
    buffer.setSize(2, 44100 * 3); // Stereo buffer, 2 seconds long
    
    // Convert the sequence to a MidiBuffer...
    mySynth.renderNextBlock(buffer, midiBuffer, 0, buffer.getNumSamples());
    
    // Write to a WAV file
    juce::File outputFile("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/output.wav");
    auto* outputStream = new juce::FileOutputStream(outputFile);
    
    if (outputStream->failedToOpen())
    {
        delete outputStream; // Don't forget to delete the stream if opening fails.
        // Handle the error - the file stream failed to open.
        return 1;
    }
    
    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer(wavFormat.createWriterFor(outputStream, 44100, buffer.getNumChannels(), 16, {}, 0));
    if (writer != nullptr) {
        writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
        // The writer now owns the outputStream and will delete it when done.
    }
    else {
        // Handle the error - writer creation failed.
        delete outputStream; // Clean up the stream if the writer wasn't created.
    }
    
    
    return 0;
}
