/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NormalEQAudioProcessor::NormalEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

NormalEQAudioProcessor::~NormalEQAudioProcessor()
{
}

//==============================================================================
const juce::String NormalEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NormalEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NormalEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NormalEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NormalEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NormalEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NormalEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NormalEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NormalEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void NormalEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NormalEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;
    
    spec.maximumBlockSize = samplesPerBlock;
    
    spec.numChannels = 1;
    
    spec.sampleRate = sampleRate;
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
}

void NormalEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NormalEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NormalEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // dsp::ProcessorChains  dsp::ProcessContextReplacing<>
    // 프로세스 체인에서는 체인의 링크를 따라 오디오를 실행하기 위해 프로세스 컨텍스트가 필요함
    // 프로세싱 컨텍스트를 만들기 위해서는 오디오 블록과 함께 전달해야 함
    
    // dsp::AudioBlock<> 인스턴스들은 juce::AudioBuffer<>와 함께 이루어진다.
    // 프로세스 블록 함수는 호스트에 의해 호출되고, 채널의 수에 따라 각각 버퍼가 주어진다.
    // 따라서 이 버퍼를 통해 좌, 우 채널로 추출한다. channel {0, 1}

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    juce::dsp::AudioBlock<float> block(buffer); // 현재 버퍼로 블록이 초기화 됨
    
    
    // audioBlock<> 클래스에서 수 많은 블록이 담긴 버퍼로부터 개별 채널을 추출할 수 있는 도우미 기능
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    // 이제 각 개별 채널 블록이 있으므로 그들을 래핑하는 컨텍스트를 만들어 줌 = 체인에서 사용 가능하도록
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    
    leftChain.process(leftContext);
    rightChain.process(rightContext);
    
    // 현재는 필터의 계수를 아무것도 설정하지 않았으므로 아무것도 하지 않는다.
}

//==============================================================================
bool NormalEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NormalEQAudioProcessor::createEditor()
{
    return new NormalEQAudioProcessorEditor (*this);
}

//==============================================================================
void NormalEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NormalEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout NormalEQAudioProcessor::createParameterLayout()
{
    // AudioProcessorParameter Diagram 참고
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq",
                                                           "LowCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
                                                           20.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
                                                           "HighCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
                                                           20000.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq",
                                                           "Peak Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
                                                           20.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain",
                                                           "Peak Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality",
                                                           "Peak Quality",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
                                                           1.f));
    
    juce::StringArray stringArray;
    for(int i = 0; i < 4; ++i)
    {
        juce::String str;
        str << (12 + i * 12);
        str << " db/Oct";
        stringArray.add(str);
    }
    
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope",
                                                            "LowCut Slope",
                                                            stringArray,
                                                            0));
    
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope",
                                                            "HighCut Slope",
                                                            stringArray,
                                                            0));
    
    
    return layout;
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NormalEQAudioProcessor();
}
