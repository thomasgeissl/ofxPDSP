
// OscInput.h
// ofxPDSP
// Nicola Pisanti, MIT License, 2016

#ifndef OFXPDSPMIDI_PDSPOSCINPUT_H_INCLUDED
#define OFXPDSPMIDI_PDSPOSCINPUT_H_INCLUDED

#include "ofMain.h"
#include <chrono>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "../DSP/pdspCore.h"
#include "../sequencer/SequencerSection.h"
#include "ofxOsc.h"

/*!
@brief utility class manage OSC input to the DSP
*/

/*!
    @cond HIDDEN_SYMBOLS
*/  
typedef osc::ReceivedMessage _PDSPOscReceivedMessage_t;
typedef osc::IpEndpointName _PDSPIpEndpointName_t;
typedef osc::osc_bundle_element_size_t _PDSPosc_bundle_element_size_t;
/*!
    @endcond
*/   

namespace pdsp{ namespace osc {

static const float Ignore = std::numeric_limits<float>::infinity(); 

class Input : public pdsp::Preparable {

private:
    //enum OscChannelMode { Gate, Value };

    class OscChannel {
    public:
        OscChannel();
        
        void deallocate();
        
        string key;
        int argument;
        pdsp::MessageBuffer* messageBuffer;
        pdsp::SequencerGateOutput* gate_out;
        pdsp::SequencerValueOutput* value_out;
        
        bool hasParser;
        std::function<float(float)> code;
    };
    
    class _PositionedOscMessage {
    public:
        _PositionedOscMessage(){ sample = -1; };
        _PositionedOscMessage(ofxOscMessage message, int sample) : message(message), sample(sample){};
        
        
        std::chrono::time_point<std::chrono::high_resolution_clock> timepoint;
        ofxOscMessage message;
        int sample;
    };

    class CustomOscReceiver : public ofxOscReceiver {
    public:
        std::vector<_PositionedOscMessage> * circularBuffer;
        std::atomic<int> * index;
    protected:
        void ProcessMessage(const _PDSPOscReceivedMessage_t &m, const _PDSPIpEndpointName_t &remoteEndpoint) override;
    };

public:
    Input();    
    ~Input();      
    

    /*!
    @brief open the port with the given index
    @param[in] port OSC port
    */    
    void openPort( int port );

    /*!
    @brief shuts down the output
    */   
    void close();


    /*!
    @brief return true if the port has been sucessfully opened
    */   
    bool isConnected() { return connected; }
    
    /*!
    @brief enable or disable diagnostic messages
    @param[in] verbose true for enabling, false for disabling
    */   
    void setVerbose( bool verbose );
    
    /*!
    @brief control the clock of the units and sequencer with an OSC message
    @param[in] oscAddress address for OSC input
    @param[in] argument index of argument from message, 0 if not gived (first argument)
    */       
    void linkTempo( string oscAddress, int argument=0 );

    /*!
    @brief get a trigger output for the given OSC address. Only the first value of those address will be taken, as float value. When you have used an address as trig output you can't use it as value
    @param[in] oscAddress address for OSC input
    @param[in] argument index of argument from message, 0 if not gived (first argument)
    */       
    pdsp::SequencerGateOutput& out_trig( string oscAddress, int argument=0 );

    /*!
    @brief get a value output for the given OSC address. Only the first value of those address will be taken, as float value. When you have used an address as value output you can't use it as trigger
    @param[in] oscAddress address for OSC input
    @param[in] argument index of argument from message, 0 if not gived (first argument)
    */    
    pdsp::SequencerValueOutput& out_value( string oscAddress, int argument=0  );


    /*!
    @brief get a reference to the parser for the selected address and argument. You can assign a lambda function to this value to elaborate the signal.
    @param[in] oscAddress address for OSC input
    @param[in] argument index of argument from message, 0 if not gived (first argument)
    */       
    std::function<float(float)> & parser( string oscAddress, int argument=0 );

    /*!
    @brief sends 0.0f as message to all the connected trigger and value outputs.
    */   
    void clearAll();

/*!
    @cond HIDDEN_SYMBOLS
*/  
    void processOsc( int bufferSize ) noexcept;
    bool hasTempoChange(); 
    double getTempo();
/*!
    @endcond
*/   

protected:
    void prepareToPlay( int expectedBufferSize, double sampleRate ) override;
    void releaseResources() override;    

private:
    CustomOscReceiver receiver;
    
    vector<OscChannel*> oscChannels;    
    
    bool            connected;
    bool            verbose;    
    
    bool    sendClearMessages; 

    
    std::atomic<int> index;
    int lastread;
    std::vector<_PositionedOscMessage>    circularBuffer;
    std::vector<_PositionedOscMessage>    readVector;


    double                                              oneSlashMicrosecForSample;

    chrono::time_point<chrono::high_resolution_clock>   bufferChrono;   

    pdsp::SequencerGateOutput invalidGate;
    pdsp::SequencerValueOutput invalidValue;

    void pushToReadVector( _PositionedOscMessage & message );
    
    bool        tempoLinked;
    std::string tempoAddress;
    int         tempoArgument;
    double      tempo;
    bool        tempoChanged;
    
};

}}

#endif // OFXPDSPMIDI_PDSPOSCINPUT_H_INCLUDED
