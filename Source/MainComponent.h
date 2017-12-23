/*
 ==============================================================================
 
 This file was auto-generated!
 
 ==============================================================================
 */

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "./MidiMessagesBoxComponent.h"
#include <math.h>


//==============================================================================
/*
 This component lives inside our window, and this is where you should put all
 your controls and content.
 */
class MainContentComponent   : public Component,
private ComboBox::Listener,
private Button::Listener,
private MidiInputCallback

{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();
    
    void paint (Graphics&) override;
    void resized() override;
    int midiChannel=10;
    
private:
    //==============================================================================
    
    AudioDeviceManager deviceManager;
    ComboBox midiInputList;
    Label midiInputListLabel;
    int lastInputIndex;
    
    MidiMessagesBoxComponent midiMessagesBoxComponent;
    double startTime;
    const File logFile=File("~/mackierelay.log");
    FileLogger *log= new FileLogger(logFile,"",128 *1024);

    //Logger::setCurrentLogger (log);
    
    //debugLogEnabled = true;
    
    
    String midiOutName_ = "IAC Driver Bus 1";
    MidiOutput* midiOut_;

    StringArray getMackieMessageDescription(const MidiMessage&);
    
    void comboBoxChanged(ComboBox*) override;
    void handleIncomingMidiMessage(MidiInput*, const MidiMessage&) override;
    void setMidiInput(int);
    void logMessage();
    void postMessageToList(const MidiMessage&, const String&);
    void addMessageToList(const MidiMessage&, const String&);
    void relayMessage(const MidiMessage&);
    bool doRelay=false;
    
    void buttonClicked (Button* button) override
    {
        int noteNumber = -1; // just used as a check that this as been set before we create a MidiMessage object
        
        if (button == &bassDrumButton)      noteNumber = 36;
        
        if (noteNumber >= 0)
        {
            MidiMessage message = MidiMessage::noteOn (midiChannel, noteNumber, (uint8) 100);
            message.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001 - startTime);
            //addMessageToList (message);
            relayMessage(message);
        }
    }
    
    // This is used to dispach an incoming message to the message thread
    class IncomingMessageCallback : public CallbackMessage
    {
    public:
        IncomingMessageCallback(MainContentComponent* o, const MidiMessage& m, const String& s)
        : owner(o), message(m), source(s)
        {}
        
        void messageCallback() override
        {
            if (owner != nullptr)
                owner->addMessageToList(message, source);
            
            owner->log->logMessage(source);
            //String strMsg=owner->getMackieMessageDescription(message).joinIntoString(", ");
            // if (!message.isSysEx()){
            
            
            //its cc
            //detect if its the plugin control button on the presonus
            if (message.isNoteOnOrOff())
            {
                
                int noteNum = message.getNoteNumber();
                //int noteVel = message.getVelocity();
                
                if (noteNum == 43){
                    // id = "ASSIGNMENT: PLUG-IN";
                    //use this to flip flag whether to relay midi or not
                    owner->doRelay=true;
                } else if (noteNum == 42){
                    //id = "ASSIGNMENT: PAN/SURROUND"; (the track button)
                    owner->doRelay=false;
                }
            } else if (message.isPitchWheel())
            {
                //channel 1-8 represent the faders from left to right
                //subtype = "Fader";
                int ch=message.getChannel();
                
                int pwv= message.getPitchWheelValue();
                if(pwv !=0){
                    pwv=sqrt(message.getPitchWheelValue());
                    if(pwv <0) pwv=0;
                }
                int midiChannel=owner->midiChannel;
                /*
                 MidiMessage newMessage = MidiMessage::controllerEvent (midiChannel, 11, pwv);
                 //newMessage.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001 - startTime);
                 owner->relayMessage(newMessage);
                 */
                
                if(ch == 1){
                    //printf("channel 1");
                    //hard code expression for now
                    MidiMessage newMessage = MidiMessage::controllerEvent (midiChannel, 11, pwv);
                    //newMessage.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001 - startTime);
                    if(owner->doRelay)owner->relayMessage(newMessage);
                } else if(ch == 2){
                    //printf("channel 2");
                    //hard code volume for now
                    MidiMessage newMessage = MidiMessage::controllerEvent (midiChannel, 7, pwv);
                    //newMessage.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001 - startTime);
                    if(owner->doRelay)owner->relayMessage(newMessage);
                }

            }

        }
        
        Component::SafePointer<MainContentComponent> owner;
        MidiMessage message;
        String source;
        
        
        //void relayMessage(MidiMessage &message);
    };
    
    TextButton bassDrumButton;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
