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
    
    // ProcessSpec 정의에서 가져옴
    // This structure is passed into a DSP algorithm's prepare() method, and contains
    // information about various aspects of the context in which it can expect to be called.
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    // 체인 세팅을 가져옴
    auto chainSettings = getChainSettings(apvts);
    

    // 정의를 보면 order의 1/2만큼의 계수를 얻게 된다
    // static ReferenceCountedArray<IIRCoefficients> designIIRHighpassHighOrderButterworthMethod \
    // (FloatType frequency, double sampleRate, int order);
    // 따라서 2 * (order + i) = 필터[i]의 계수 값
    // 피크의 계수를 설정
    auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
                                                                                                       getSampleRate(),
                                                                                                       2 * (chainSettings.lowCutSlope) + 1);
    auto& leftLowCut = leftChain.get<ChainPosition::LowCut>();
    updateCutFilter(leftLowCut, cutCoefficients, chainSettings.lowCutSlope);
    
    auto& rightLowCut = rightChain.get<ChainPosition::LowCut>();
    updateCutFilter(rightLowCut, cutCoefficients, chainSettings.lowCutSlope);
    
    
    /* move to updatePeakFilter()
    
     
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                                                chainSettings.peakFreq,
                                                                                chainSettings.peakQuality,
                                                                                juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
    
    // chain get 함수는 체인이 가지는 인덱스를 필요로 한다.
    // enum을 통해 인덱스를 정의해서 전달하는데, 역참조임을 유의(오디오 콜백 힙에 할당하는 것이 좋지 않은 디자인이라고)
    *leftChain.get<ChainPosition::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPosition::Peak>().coefficients = *peakCoefficients;
     
    */
    
    //auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,sampleRate, 2 * (chainSettings.lowCutSlope) + 1);
    //auto& leftLowCut = leftChain.get<ChainPosition::LowCut>();
    
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
    
    
    // PrepareToPlay에서는 슬라이더 값 변경 시 새로운 계수로 업데이트되지 않기 때문에,
    // 프로세스 블록에서 오디오 처리 전 항상 매개변수를 변경하기 위해 버퍼 블록 초기화 이전에 삽입
    auto chainSettings = getChainSettings(apvts);

    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                                chainSettings.peakFreq,
                                                                                chainSettings.peakQuality,
                                                                                juce::Decibels::decibelsToGain
                                                                                (chainSettings.peakGainInDecibels));
    
    updatePeakFilter(chainSettings);
    /*
    *leftChain.get<ChainPosition::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPosition::Peak>().coefficients = *peakCoefficients;
    */
     
    auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
                                                                                                       getSampleRate(),
                                                                                                       2 * (chainSettings.lowCutSlope) + 1);
    auto& leftLowCut = leftChain.get<ChainPosition::LowCut>();
    updateCutFilter(leftLowCut, cutCoefficients, chainSettings.lowCutSlope);
    
    auto& rightLowCut = rightChain.get<ChainPosition::LowCut>();
    updateCutFilter(rightLowCut, cutCoefficients, chainSettings.lowCutSlope);
    
    
    /* move to updateCoefficents
     
    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);
    
    switch( chainSettings.lowCutSlope )
    {
        case Slope_12:
        {
            // 역참조
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            break;
        }
        case Slope_36:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            *leftLowCut.get<2>().coefficients = *cutCoefficients[2];
            leftLowCut.setBypassed<2>(false);
            break;
        }
        case Slope_48:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            *leftLowCut.get<2>().coefficients = *cutCoefficients[2];
            leftLowCut.setBypassed<2>(false);
            *leftLowCut.get<3>().coefficients = *cutCoefficients[3];
            leftLowCut.setBypassed<3>(false);
            break;
        }
    }
    
    
    auto& rightLowCut = rightChain.get<ChainPosition::LowCut>();
    
    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);
    
    switch( chainSettings.lowCutSlope )
    {
        case Slope_12:
        {
            // 역참조
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            break;
        }
        case Slope_36:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            *rightLowCut.get<2>().coefficients = *cutCoefficients[2];
            rightLowCut.setBypassed<2>(false);
            break;
        }
        case Slope_48:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            *rightLowCut.get<2>().coefficients = *cutCoefficients[2];
            rightLowCut.setBypassed<2>(false);
            *rightLowCut.get<3>().coefficients = *cutCoefficients[3];
            rightLowCut.setBypassed<3>(false);
            break;
        }
    }
    */
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
    //return new NormalEQAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
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

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    // thread의 안전 상 이슈가 존재하지만 여기선 다루지 않음
    // apvts.getParameter("LowCut Freq")->getValue(); 이 경우 정규화된 값이 주어지기 때문에 사용 x
    
    // 아래 파라미터 레이아웃에서 설정했던 파라미터 값을 받을 수 있게 됨
    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
    settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
    
    // Slope로 명시적 형변환
    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());
    
    return settings;
}


void NormalEQAudioProcessor::updatePeakFilter(const ChainSettings &chainSettings)
{
    // 피크의 계수를 설정
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                                chainSettings.peakFreq,
                                                                                chainSettings.peakQuality,
                                                                                juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
    
    // chain get 함수는 체인이 가지는 인덱스를 필요로 한다.
    // enum을 통해 인덱스를 정의해서 전달하는데, 역참조임을 유의(오디오 콜백 힙에 할당하는 것이 좋지 않은 디자인이라고)
    // *leftChain.get<ChainPosition::Peak>().coefficients = *peakCoefficients;
    // *rightChain.get<ChainPosition::Peak>().coefficients = *peakCoefficients;
    
    // 리팩토링
    updateCoefficients(leftChain.get<ChainPosition::Peak>().coefficients, peakCoefficients);
    updateCoefficients(rightChain.get<ChainPosition::Peak>().coefficients, peakCoefficients);
}

void NormalEQAudioProcessor::updateCoefficients(Coefficients &old, const Coefficients &replacements)
{
    //
    *old = *replacements;
}

juce::AudioProcessorValueTreeState::ParameterLayout NormalEQAudioProcessor::createParameterLayout()
{
    // AudioProcessorParameter Diagram 참고
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq",
                                                           "LowCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           20.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
                                                           "HighCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           20000.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq",
                                                           "Peak Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
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
