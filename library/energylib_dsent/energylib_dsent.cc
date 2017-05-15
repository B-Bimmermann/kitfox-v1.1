#include <iostream>
#include <cmath>
#include "kitfox.h"
#include "energylib_dsent.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;
using namespace libKitFox_DSENT;
using namespace DSENT;

energylib_dsent::energylib_dsent(pseudo_component_t *PseudoComponent) :
    energy_library_t(PseudoComponent,KITFOX_LIBRARY_MODEL_DSENT),
    //dsent_config(NULL),
    tech_model(NULL),
    std_cell_lib(NULL),
    ms_model(NULL),
    energy_model(KITFOX_DSENT_ENERGY_MODEL_UNKNOWN),
    energy_scaling(1.0),
    area_scaling(1.0),
    scaling(1.0),
    tdp_injection_rate(0.3),
    IsPerformTimingOptimization(false)
{}

energylib_dsent::~energylib_dsent() {
    //delete dsent_config;
    delete tech_model;
    delete ms_model;
    start_net_names.clear();
}


void energylib_dsent::initialize() {
    try {
        //DSENTConfig *dsent_config = new DSENTConfig();
        tech_model = new TechModel();
        
        clock_frequency = pseudo_component->lookup_in_library("clock_frequency",true);
        
        if(pseudo_component->exists_in_library("TDPInjectionRate"))
            tdp_injection_rate = pseudo_component->lookup_in_library("TDPInjectionRate");
        else
            tdp_injection_rate = 0.3;
            
        if(pseudo_component->exists_in_library("energy_scaling",true))
            energy_scaling = pseudo_component->lookup_in_library("energy_scaling",true);
        else
            energy_scaling = 1.0;
            
        if(pseudo_component->exists_in_library("area_scaling",true))
            area_scaling = pseudo_component->lookup_in_library("area_scaling",true);
        else
            area_scaling = 1.0;
            
        // Scaling factor applies to area and leakage energy.
        // Scaling factor is used for duplicated components.
        // For instance, if there are two routers to be modeled,
        // they can be configured as Router0 and Router1,
        // or simply Router with scaling = 2 such that area and leakage power are doubled.
        // Note that dynamic energy is not scaled, otherwise 1 counter access represents
        // simultaneous accesses to both routers which may not be the intended case.
        if(pseudo_component->exists_in_library("scaling",true))
            scaling = pseudo_component->lookup_in_library("scaling",true);
        else
            scaling = 1.0;
        
        // This is the part of TechModel::readFile(const String& filename_)
        const char *tech_model_str = pseudo_component->lookup_in_library("tech_model",true);
        tech_model->set(String("Name"),String(tech_model_str));
        
        tech_model->set(String("Vdd"),ConvertString((double)pseudo_component->lookup_in_library("voltage",true)));
        tech_model->set(String("Temperature"),ConvertString((double)pseudo_component->lookup_in_library("temperature",true)));
        
        Setting &gate_setting = pseudo_component->lookup_in_library("Gate",true);
        tech_model->set(String("Gate->PitchContacted"),ConvertString((double)gate_setting["PitchContacted"]));
        tech_model->set(String("Gate->MinWidth"),ConvertString((double)gate_setting["MinWidth"]));
        tech_model->set(String("Gate->CapPerWidth"),ConvertString((double)gate_setting["CapPerWidth"]));
        
        Setting &drain_setting = pseudo_component->lookup_in_library("Drain",true);
        tech_model->set(String("Drain->CapPerWidth"),ConvertString((double)drain_setting["CapPerWidth"]));
        
        Setting &nmos_setting = pseudo_component->lookup_in_library("Nmos",true);
        tech_model->set(String("Nmos->CharacterizedTemperature"),ConvertString((double)nmos_setting["CharacterizedTemperature"]));
        tech_model->set(String("Nmos->EffResWidth"),ConvertString((double)nmos_setting["EffResWidth"]));
        tech_model->set(String("Nmos->EffResStackRatio"),ConvertString((double)nmos_setting["EffResStackRatio"]));
        tech_model->set(String("Nmos->OffCurrent"),ConvertString((double)nmos_setting["OffCurrent"]));
        tech_model->set(String("Nmos->MinOffCurrent"),ConvertString((double)nmos_setting["MinOffCurrent"]));
        tech_model->set(String("Nmos->SubthresholdSwing"),ConvertString((double)nmos_setting["SubthresholdSwing"]));
        tech_model->set(String("Nmos->DIBL"),ConvertString((double)nmos_setting["DIBL"]));
        tech_model->set(String("Nmos->SubthresholdTempSwing"),ConvertString((double)nmos_setting["SubthresholdTempSwing"]));
        
        Setting &pmos_setting = pseudo_component->lookup_in_library("Pmos",true);
        tech_model->set(String("Pmos->CharacterizedTemperature"),ConvertString((double)pmos_setting["CharacterizedTemperature"]));
        tech_model->set(String("Pmos->EffResWidth"),ConvertString((double)pmos_setting["EffResWidth"]));
        tech_model->set(String("Pmos->EffResStackRatio"),ConvertString((double)pmos_setting["EffResStackRatio"]));
        tech_model->set(String("Pmos->OffCurrent"),ConvertString((double)pmos_setting["OffCurrent"]));
        tech_model->set(String("Pmos->MinOffCurrent"),ConvertString((double)pmos_setting["MinOffCurrent"]));
        tech_model->set(String("Pmos->SubthresholdSwing"),ConvertString((double)pmos_setting["SubthresholdSwing"]));
        tech_model->set(String("Pmos->DIBL"),ConvertString((double)pmos_setting["DIBL"]));
        tech_model->set(String("Pmos->SubthresholdTempSwing"),ConvertString((double)pmos_setting["SubthresholdTempSwing"]));
        
        Setting &wire_setting = pseudo_component->lookup_in_library("Wire",true);
        tech_model->m_available_wire_layers_ = new set<String>;
        for(unsigned i = 0; i < wire_setting["available_layers"].getLength(); i++) {
            const char *wire_layer_str = wire_setting["available_layers"][i];
            tech_model->m_available_wire_layers_->insert(String(wire_layer_str));
            
            Setting &layer_wire_setting = wire_setting[wire_layer_str];
            tech_model->set(String("Wire->")+String(layer_wire_setting.getName())+String("->MinWidth"),ConvertString((double)layer_wire_setting["MinWidth"]));
            tech_model->set(String("Wire->")+String(layer_wire_setting.getName())+String("->MinSpacing"),ConvertString((double)layer_wire_setting["MinSpacing"]));
            tech_model->set(String("Wire->")+String(layer_wire_setting.getName())+String("->Resistivity"),ConvertString((double)layer_wire_setting["Resistivity"]));
            tech_model->set(String("Wire->")+String(layer_wire_setting.getName())+String("->MetalThickness"),ConvertString((double)layer_wire_setting["MetalThickness"]));
            tech_model->set(String("Wire->")+String(layer_wire_setting.getName())+String("->DielectricThickness"),ConvertString((double)layer_wire_setting["DielectricThickness"]));
            tech_model->set(String("Wire->")+String(layer_wire_setting.getName())+String("->DielectricConstant"),ConvertString((double)layer_wire_setting["DielectricConstant"]));
        }
        
        Setting &stdcell_setting = pseudo_component->lookup_in_library("StdCell",true);
        tech_model->set(String("StdCell->Tracks"),ConvertString((double)stdcell_setting["Tracks"]));
        tech_model->set(String("StdCell->HeightOverheadFactor"),ConvertString((double)stdcell_setting["HeightOverheadFactor"]));
        
        char buf[256];
        sprintf(buf,"[");
        for(unsigned i = 0; i < stdcell_setting["AvailableSizes"].getLength(); i++) {
            if(i) strcat(buf,",");
            strcat(buf,ConvertString((double)stdcell_setting["AvailableSizes"][i]).c_str());
        }
        strcat(buf,"]");
        tech_model->set(String("StdCell->AvailableSizes"),String(buf));

        if(!strcasecmp(tech_model_str,"Photonics")) { // Photonic tech models
            Setting &waveguide_setting = pseudo_component->lookup_in_library("Waveguide",true);
            tech_model->set(String("Waveguide->LossPerMeter"),ConvertString((double)waveguide_setting["LossPerMeter"]));
            tech_model->set(String("Waveguide->Pitch"),ConvertString((double)waveguide_setting["Pitch"]));
            
            Setting &laser_setting = pseudo_component->lookup_in_library("Laser",true);
            Setting &cw_setting = laser_setting["CW"];
            tech_model->set(String("Laser->CW->Efficiency"),ConvertString((double)cw_setting["Efficiency"]));
            tech_model->set(String("Laser->CW->LaserDiodeLoss"),ConvertString((double)cw_setting["LaserDiodeLoss"]));
            tech_model->set(String("Laser->CW->Area"),ConvertString((double)cw_setting["Area"]));
            Setting &gated_cw_setting = laser_setting["GatedCW"];
            tech_model->set(String("Laser->GatedCW->Efficiency"),ConvertString((double)gated_cw_setting["Efficiency"]));
            tech_model->set(String("Laser->GatedCW->LaserDiodeLoss"),ConvertString((double)gated_cw_setting["LaserDiodeLoss"]));
            tech_model->set(String("Laser->GatedCW->Area"),ConvertString((double)gated_cw_setting["Area"]));
            
            Setting &modulator_setting = pseudo_component->lookup_in_library("Modulator",true);
            Setting &ring_setting = modulator_setting["Ring"];
            tech_model->set(String("Modulator->Ring->SupplyBoostRatio"),ConvertString((double)ring_setting["SupplyBoostRatio"]));
            tech_model->set(String("Modulator->Ring->ParasiticRes"),ConvertString((double)ring_setting["ParasiticRes"]));
            tech_model->set(String("Modulator->Ring->ParasiticCap"),ConvertString((double)ring_setting["ParasiticCap"]));
            tech_model->set(String("Modulator->Ring->FCPDEffect"),ConvertString((double)ring_setting["FCPDEffect"]));
            tech_model->set(String("Modulator->Ring->Tn"),ConvertString((double)ring_setting["Tn"]));
            tech_model->set(String("Modulator->Ring->NA"),ConvertString((double)ring_setting["NA"]));
            tech_model->set(String("Modulator->Ring->ND"),ConvertString((double)ring_setting["ND"]));
            tech_model->set(String("Modulator->Ring->ni"),ConvertString((double)ring_setting["ni"]));
            tech_model->set(String("Modulator->Ring->JunctionRatio"),ConvertString((double)ring_setting["JunctionRatio"]));
            tech_model->set(String("Modulator->Ring->Height"),ConvertString((double)ring_setting["Height"]));
            tech_model->set(String("Modulator->Ring->Width"),ConvertString((double)ring_setting["Width"]));
            tech_model->set(String("Modulator->Ring->ConfinementFactor"),ConvertString((double)ring_setting["ConfinementFactor"]));
            
            Setting &resonator_setting = pseudo_component->lookup_in_library("Ring",true);
            tech_model->set(String("Ring->Area"),ConvertString((double)resonator_setting["Area"]));
            tech_model->set(String("Ring->Lambda"),ConvertString((double)resonator_setting["Lambda"]));
            tech_model->set(String("Ring->GroupIndex"),ConvertString((double)resonator_setting["GroupIndex"]));
            tech_model->set(String("Ring->Radius"),ConvertString((double)resonator_setting["Radius"]));
            tech_model->set(String("Ring->ConfinementFactor"),ConvertString((double)resonator_setting["ConfinementFactor"]));
            tech_model->set(String("Ring->ThroughLoss"),ConvertString((double)resonator_setting["ThroughLoss"]));
            tech_model->set(String("Ring->DropLoss"),ConvertString((double)resonator_setting["DropLoss"]));
            tech_model->set(String("Ring->MaxQualityFactor"),ConvertString((double)resonator_setting["MaxQualityFactor"]));
            tech_model->set(String("Ring->HeatingEfficiency"),ConvertString((double)resonator_setting["HeatingEfficiency"]));
            tech_model->set(String("Ring->TuningEfficiency"),ConvertString((double)resonator_setting["TuningEfficiency"]));
            tech_model->set(String("Ring->LocalVariationSigma"),ConvertString((double)resonator_setting["LocalVariationSigma"]));
            tech_model->set(String("Ring->SystematicVariationSigma"),ConvertString((double)resonator_setting["SystematicVariationSigma"]));
            tech_model->set(String("Ring->TemperatureMax"),ConvertString((double)resonator_setting["TemperatureMax"]));
            tech_model->set(String("Ring->TemperatureMin"),ConvertString((double)resonator_setting["TemperatureMin"]));
            tech_model->set(String("Ring->MaxElectricallyTunableFreq"),ConvertString((double)resonator_setting["MaxElectricallyTunableFreq"]));
            
            Setting &photodetector_setting = pseudo_component->lookup_in_library("Photodetector",true);
            tech_model->set(String("Photodetector->Responsivity"),ConvertString((double)photodetector_setting["Responsivity"]));
            tech_model->set(String("Photodetector->Area"),ConvertString((double)photodetector_setting["Area"]));
            tech_model->set(String("Photodetector->Cap"),ConvertString((double)photodetector_setting["Cap"]));
            tech_model->set(String("Photodetector->ParasiticCap"),ConvertString((double)photodetector_setting["ParasiticCap"]));
            tech_model->set(String("Photodetector->Loss"),ConvertString((double)photodetector_setting["Loss"]));
            tech_model->set(String("Photodetector->MinExtinctionRatio"),ConvertString((double)photodetector_setting["MinExtinctionRatio"]));
            tech_model->set(String("photodetector->AvalancheGain"),ConvertString((double)photodetector_setting["AvalancheGain"]));
            
            Setting &sense_amp_setting = pseudo_component->lookup_in_library("SenseAmp",true);
            tech_model->set(String("SenseAmp->BER"),ConvertString((double)sense_amp_setting["BER"]));
            tech_model->set(String("SenseAmp->CMRR"),ConvertString((double)sense_amp_setting["CMRR"]));
            tech_model->set(String("SenseAmp->OffstCompensationBits"),ConvertString((double)sense_amp_setting["OffsetCompensationBits"]));
            tech_model->set(String("SenseAmp->OffsetRatio"),ConvertString((double)sense_amp_setting["OffsetRatio"]));
            tech_model->set(String("SenseAmp->SupplyNoiseRandRatio"),ConvertString((double)sense_amp_setting["SupplyNoiseRandRatio"]));
            tech_model->set(String("SenseAmp->SupplyNoiseDetRatio"),ConvertString((double)sense_amp_setting["SupplyNoiseDetRatio"]));
            tech_model->set(String("SenseAmp->NoiseMargin"),ConvertString((double)sense_amp_setting["NoiseMargin"]));
            tech_model->set(String("SenseAmp->JitterRatio"),ConvertString((double)sense_amp_setting["JitterRatio"]));
            
            tech_model->set(String("Receiver->Int->IntegrationTimeRatio"),ConvertString((double)pseudo_component->lookup_in_library("Receiver",true)["Int"]["IntegrationTimeRatio"]));
        }
        
        // This is the part of DSENTConfig::constructTechModel(const String& overwrite_str_)
        // Allocate static StdCellLib instance
        StdCellLib* std_cell_lib = new StdCellLib(tech_model);
        
        // Set the StdCellLib pointer in static TechModel instance
        tech_model->setStdCellLib(std_cell_lib);
        
        //dsent_config->m_tech_model_ = tech_model;
        
        // This is the part of DSENT::buildModel()
        const char *energy_model_str = pseudo_component->lookup_in_library("energy_model",true);
        ms_model = ModelGen::createModel(String(energy_model_str),String(energy_model_str),tech_model);
        
        if(!strcasecmp(energy_model_str,"Router")) {
            energy_model = KITFOX_DSENT_ROUTER;
            
            ms_model->setParameter(String("Frequency"),ConvertString(clock_frequency));
            ms_model->setParameter(String("NumberInputPorts"),ConvertString((int)pseudo_component->lookup_in_library("NumberInputPorts")));
            ms_model->setParameter(String("NumberOutputPorts"),ConvertString((int)pseudo_component->lookup_in_library("NumberOutputPorts")));
            ms_model->setParameter(String("NumberBitsPerFlit"),ConvertString((int)pseudo_component->lookup_in_library("NumberBitsPerFlit")));
            ms_model->setParameter(String("NumberVirtualNetworks"),ConvertString((int)pseudo_component->lookup_in_library("NumberVirtualNetworks")));
            
            char buf[256];
            sprintf(buf,"[");
            for(unsigned i = 0; i < pseudo_component->lookup_in_library("NumberVirtualChannelsPerVirtualNetwork").getLength(); i++) {
                if(i) strcat(buf,",");
                strcat(buf,ConvertString((int)pseudo_component->lookup_in_library("NumberVirtualChannelsPerVirtualNetwork")[i]));
            }
            strcat(buf,"]");
            ms_model->setParameter(String("NumberVirtualChannelsPerVirtualNetwork"),String(buf));
            
            sprintf(buf,"[");
            for(unsigned i = 0; i < pseudo_component->lookup_in_library("NumberBuffersPerVirtualChannel").getLength(); i++) {
                if(i) strcat(buf,",");
                strcat(buf,ConvertString((int)pseudo_component->lookup_in_library("NumberBuffersPerVirtualChannel")[i]));
            }
            strcat(buf,"]");
            ms_model->setParameter(String("NumberBuffersPerVirtualChannel"),String(buf));

            ms_model->setParameter(String("InputPort->BufferModel"),(const char*)pseudo_component->lookup_in_library("InputPort")["BufferModel"]);
            ms_model->setParameter(String("CrossbarModel"),(const char*)pseudo_component->lookup_in_library("CrossbarModel"));
            ms_model->setParameter(String("SwitchAllocator->ArbiterModel"),(const char*)pseudo_component->lookup_in_library("SwitchAllocator")["ArbiterModel"]);
            ms_model->setParameter(String("ClockTreeModel"),(const char*)pseudo_component->lookup_in_library("ClockTreeModel"));
            ms_model->setParameter(String("ClockTree->NumberLevels"),ConvertString((int)pseudo_component->lookup_in_library("ClockTree")["NumberLevels"]));
            ms_model->setParameter(String("ClockTree->WireLayer"),(const char*)pseudo_component->lookup_in_library("ClockTree")["WireLayer"]);
            ms_model->setParameter(String("ClockTree->WireWidthMultiplier"),ConvertString((double)pseudo_component->lookup_in_library("ClockTree")["WireWidthMultiplier"]));
        }
        else if(!strcasecmp(energy_model_str,"RepeatedLink")) {
            energy_model = KITFOX_DSENT_REPEATED_LINK;
            
            ms_model->setParameter(String("NumberBits"),ConvertString((int)pseudo_component->lookup_in_library("NumberBits")));
            ms_model->setParameter(String("WireLayer"),(const char*)pseudo_component->lookup_in_library("WireLayer"));
            ms_model->setParameter(String("WireWidthMultiplier"),ConvertString((double)pseudo_component->lookup_in_library("WireWidthMultiplier")));
            ms_model->setParameter(String("WireSpacingMultiplier"),ConvertString((double)pseudo_component->lookup_in_library("WireSpacingMultiplier")));
            ms_model->setProperty(String("WireLength"),ConvertString((double)pseudo_component->lookup_in_library("WireLength")));
            ms_model->setProperty(String("Delay"),ConvertString((double)pseudo_component->lookup_in_library("Delay")));
        }
        else if(!strcasecmp(energy_model_str,"SWSRLink")) {
            energy_model = KITFOX_DSENT_SWSRLINK;
            
            pseudo_component->kitfox->error("dsent","%s.library.energy_model = %s is not supported",pseudo_component->name.c_str(),KITFOX_DSENT_ENERGY_MODEL_STR[energy_model]);
        }
        else {
            pseudo_component->kitfox->error("dsent","%s.library.energy_model = %s is unknown",pseudo_component->name.c_str(),energy_model_str);
        }
        
        /*const vector<String> *parameter_names = ms_model->getParameterNames();
        for(vector<String>::const_iterator it = parameter_names->begin(); it != parameter_names->end(); it++) {
            const String &parameter_name = *it;
            if(dsent_config->keyExist(parameter_name)) {
                ms_model->setParameter(parameter_name,dsent_config->get(parameter_name));
            }
        }*/
        ms_model->construct();
        
        /*const vector<String> *property_names = ms_model->getPropertyNames();
        for(vector<String>::const_iterator it = property_names->begin(); it != property_names->end(); it++) {
            const String &property_name = *it;
            if(dsent_config->keyExist(property_name)) {
                ms_model->setProperty(property_name,dsent_config->get(property_name));
            }
        }*/
        ms_model->update();
        
        if(pseudo_component->exists_in_library("IsPerformTimingOptimization")&&
           ((bool)pseudo_component->lookup_in_library("IsPerformTimingOptimization") == true)) {
            IsPerformTimingOptimization = true;
            Setting &timing_optimization_setting = pseudo_component->lookup_in_library("TimingOptimization",true);
            for(unsigned i = 0; i < timing_optimization_setting["StartNetNames"].getLength(); i++) {
                start_net_names.push_back(String((const char*)timing_optimization_setting["StartNetNames"][i]));
            }
            if(start_net_names.size() == 0) {
                ElectricalModel *electrical_model = (ElectricalModel*)ms_model;
                
                ElectricalTimingOptimizer timing_optimizer("Optimizer",electrical_model->getTechModel());
                timing_optimizer.setModel(electrical_model);
                timing_optimizer.construct();
                timing_optimizer.update();
                
                ElectricalTimingTree timing_tree(timing_optimizer.getInstanceName(),&timing_optimizer);
                
                const Map<PortInfo*> *input_ports = timing_optimizer.getInputs();
                Map<PortInfo*>::ConstIterator it_begin = input_ports->begin();
                Map<PortInfo*>::ConstIterator it_end = input_ports->end();
                Map<PortInfo*>::ConstIterator it;
                for(it = it_begin; it != it_end; it++) {
                    const String &net_name = it->first;
                    timing_tree.performTimingOpt(timing_optimizer.getNet(net_name,makeNetIndex(0)),1.0/clock_frequency);
                }
            }
            else {
                ElectricalModel *electrical_model = (ElectricalModel*)ms_model;
                ElectricalTimingTree timing_tree(electrical_model->getInstanceName(),electrical_model);
                for(unsigned i = 0; i < start_net_names.size(); i++) {
                    const String &net_name = start_net_names[i];
                    timing_tree.performTimingOpt(electrical_model->getNet(net_name),1.0/clock_frequency);
                }
            }
        }
        ms_model->evaluate();
        
        /*if(pseudo_component->exists_in_library("IsReportTiming")&&
            ((bool)pseudo_component->lookup_in_library("IsReportTiming") == true)) {
            IsReportTiming = true;
            Setting &report_timing_setting = pseudo_component->lookup_in_library("ReportTiming");
            vector<String> start_net_names;
            for(unsigned i = 0; i < report_timing_setting["StartNetNames"].getLength(); i++) {
                start_net_names.push_back(String((const char*)report_timing_setting["StartNetNames"][i]));
            }
            ElectricalModel *electrical_model = (ElectricalModel*)ms_model;
            ElectricalTimingTree timing_tree(electrical_model->getInstanceName(),electrical_model);
            for(unsigned i = 0; i < start_net_names.size(); i++) {
                const String &net_name = start_net_names[i];
                double timing = timing_tree.performCritPathExtract(electrical_model->getNet(net_name));
            }
        }*/
    }
    catch(SettingTypeException e) {
        cout << "KITFOX ERROR (libconfig): " << e.getPath() << " has incorrect type" << endl;
        exit(EXIT_FAILURE);
    }
}

unit_energy_t energylib_dsent::get_unit_energy() {
    unit_energy_t unit_energy;
    
    switch(energy_model) {
        case KITFOX_DSENT_ROUTER: {
            // Buffer
            unit_energy.read += ((Result*)ms_model->parseQuery(String("Energy"),String("Router"),String("ReadBuffer")))->calculateSum();
            unit_energy.write += ((Result*)ms_model->parseQuery(String("Energy"),String("Router"),String("WriteBuffer")))->calculateSum();
            //unit_energy.leakage += ((Result*)ms_model->parseQuery(String("NddPower"),String("Router->InputPort"),String("Leakage")))->calculateSum() * ms_model->getParameter(String("NumberInputPorts")).toDouble() + (((Result*)ms_model->parseQuery(String("NddPower"),String("Router->PipelineReg0"),String("Leakage")))->calculateSum() + ((Result*)ms_model->parseQuery(String("NddPower"),String("Router->PipelineReg1"),String("Leakage")))->calculateSum()) * ms_model->getParameter(String("NumberInputPorts")).toDouble() * ms_model->getParameter(String("NumberBitsPerFlit")).toDouble();
            // Crossbar
            unit_energy.switching += ((Result*)ms_model->parseQuery(String("Energy"),String("Router"),String("TraverseCrossbar->Multicast1")))->calculateSum();
            //unit_energy.leakage += ((Result*)ms_model->parseQuery(String("NddPower"),String("Router->Crossbar"),String("Leakage")))->calculateSum() + ((Result*)ms_model->parseQuery(String("NddPower"),String("Router->PipelineReg2_0"),String("Leakage")))->calculateSum() * ms_model->getParameter(String("NumberOutputPorts")).toDouble() * ms_model->getParameter(String("NumberBitsPerFlit")).toDouble();
            // Switch
            unit_energy.switching += ((Result*)ms_model->parseQuery(String("Energy"),String("Router"),String("ArbitrateSwitch->ArbitrateStage1")))->calculateSum() + ((Result*)ms_model->parseQuery(String("Energy"),String("Router"),String("ArbitrateSwitch->ArbitrateStage2")))->calculateSum();
            //unit_energy.leakage += ((Result*)ms_model->parseQuery(String("NddPower"),String("Router->SwitchAllocator"),String("Leakage")))->calculateSum();
            // Clock
            unit_energy.baseline += ((Result*)ms_model->parseQuery(String("Energy"),String("Router"),String("DistributeClock")))->calculateSum();
            //unit_energy.leakage += ((Result*)ms_model->parseQuery(String("NddPower"),String("Router->ClockTree"),String("Leakage")))->calculateSum();
            unit_energy.leakage = ((Result*)ms_model->parseQuery(String("NddPower"),String("Router"),String("Leakage")))->calculateSum();
            break;
        }
        case KITFOX_DSENT_REPEATED_LINK: {
            unit_energy.switching = ((Result*)ms_model->parseQuery(String("Energy"),String("RepeatedLink"),String("Send")))->calculateSum();
            unit_energy.leakage = ((Result*)ms_model->parseQuery(String("NddPower"),String("RepeatedLink"),String("Leakage")))->calculateSum();
            break;
        }
        default: break;
    }
    
    unit_energy.leakage /= clock_frequency;

    /*cout << "unit_energy.baseline=" << unit_energy.baseline << endl;
    cout << "unit_energy.read=" << unit_energy.read << endl;
    cout << "unit_energy.write=" << unit_energy.write << endl;
    cout << "unit_energy.switching=" << unit_energy.switching << endl;
    cout << "unit_energy.leakage=" << unit_energy.leakage << endl;*/
    
    unit_energy.baseline *= energy_scaling;
    unit_energy.switching *= energy_scaling;
    unit_energy.read *= energy_scaling;
    unit_energy.write *= energy_scaling;
    unit_energy.leakage *= (energy_scaling*scaling);
    
    return unit_energy;
}

power_t energylib_dsent::get_tdp_power(Kelvin MaxTemperature) {
    Kelvin current_temperature = tech_model->get("Temperature");
    Kelvin max_temperature = MaxTemperature;
    update_library_variable(KITFOX_DATA_TEMPERATURE,&max_temperature,false);

    unit_energy_t unit_energy = get_unit_energy();
    energy_t tdp_energy;
    power_t tdp_power;

    switch(energy_model) {
        case KITFOX_DSENT_ROUTER: {
            // R/W Buffer
            tdp_energy.dynamic += unit_energy.read * ms_model->getParameter("NumberOutputPorts").toDouble();
            tdp_energy.dynamic += unit_energy.write * ms_model->getParameter("NumberInputPorts").toDouble();
            // Switch + Crossbar
            tdp_energy.dynamic += unit_energy.switching * ms_model->getParameter("NumberOutputPorts").toDouble();
            
            tdp_energy.dynamic *= tdp_injection_rate;
            
            // Clock
            tdp_energy.dynamic += unit_energy.baseline;
            break;
        }
        case KITFOX_DSENT_REPEATED_LINK: {
            tdp_energy.dynamic *= tdp_injection_rate;
            break;
        }
        default: break;
    }
    
    tdp_energy.leakage = unit_energy.leakage;
    tdp_energy.total = tdp_energy.get_total();
    tdp_power = tdp_energy*clock_frequency;
    
    /*cout << "tdp_power.dynamic=" << tdp_power.dynamic << endl;
    cout << "tdp_power.leakage=" << tdp_power.leakage << endl;
    cout << "tdp_power.total=" << tdp_power.get_total() << endl;*/
    
    update_library_variable(KITFOX_DATA_TEMPERATURE,&current_temperature,false);
    
    return tdp_power;
}

power_t energylib_dsent::get_runtime_power(Second Time, Second Period, counter_t Counter) {
    unit_energy_t unit_energy = get_unit_energy();
    energy_t runtime_energy;
    power_t runtime_power;
    
    runtime_energy.dynamic = unit_energy.baseline*Period*clock_frequency;
    runtime_energy.dynamic += unit_energy.switching*(double)Counter.switching;
    runtime_energy.dynamic += unit_energy.read*(double)Counter.read;
    runtime_energy.dynamic += unit_energy.write*(double)Counter.write;
    runtime_energy.leakage = unit_energy.leakage*Period*clock_frequency;
    runtime_energy.total = runtime_energy.get_total();
    runtime_power = runtime_energy/Period;
    
#ifdef KITFOX_DEBUG
    cout << "KITFOX DEBUG (dsent): pseudo component " << pseudo_component->name << "[ID=" << pseudo_component->id << "].runtime_power = " << runtime_power.get_total() << "W (energy = " << runtime_energy.get_total() << "J)" << endl;
    
    double baseline_energy = unit_energy.baseline*Period*clock_frequency;
    if(unit_energy.baseline > 0.0) cout << "\t\tbaseline (counterless) = " << baseline_energy/Period << " W (energy = " << baseline_energy << "J)" << endl;
    
    double read_energy = unit_energy.read*(double)Counter.read;
    if(unit_energy.read > 0.0) cout << "\t\tread = " << read_energy/Period << "W (energy = " << read_energy << "J)" << endl;
    
    double write_energy = unit_energy.write*(double)Counter.write;
    if(unit_energy.write > 0.0) cout << "\t\twrite = " << write_energy/Period << "W (energy = " << write_energy << "J)" << endl;
    
    double switching_energy = unit_energy.switching*(double)Counter.switching;
    if(unit_energy.switching > 0.0) cout << "\t\tswitching = " << switching_energy/Period << "W (energy = " << switching_energy << "J)" << endl;
    
    double leakage_energy = unit_energy.leakage*Period*clock_frequency;
    if(unit_energy.leakage > 0.0) cout << "\t\tleakage = " << leakage_energy/Period << "W (energy = " << leakage_energy << "J)" << endl;
#endif
    
    return runtime_power;
}

MeterSquare energylib_dsent::get_area() {
    MeterSquare area = 0.0;
    
    switch(energy_model) {
        case KITFOX_DSENT_ROUTER: {
            area = (((Result*)ms_model->parseQuery(String("Area"),String("Router->InputPort"),String("Active")))->calculateSum() + (((Result*)ms_model->parseQuery(String("Area"),String("Router->PipelineReg0"),String("Active")))->calculateSum() + ((Result*)ms_model->parseQuery(String("Area"),String("Router->PipelineReg1"),String("Active")))->calculateSum()) * ms_model->getParameter(String("NumberBitsPerFlit")).toDouble()) * ms_model->getParameter(String("NumberInputPorts")).toDouble();
            area += ((Result*)ms_model->parseQuery(String("Area"),String("Router->Crossbar"),String("Active")))->calculateSum() + ((Result*)ms_model->parseQuery(String("Area"),String("Router->Crossbar_Sel_DFF"),String("Active")))->calculateSum() + ((Result*)ms_model->parseQuery(String("Area"),String("Router->PipelineReg2_0"),String("Active")))->calculateSum() * ms_model->getParameter(String("NumberBitsPerFlit")).toDouble() * ms_model->getParameter(String("NumberOutputPorts")).toDouble();
            area += ((Result*)ms_model->parseQuery(String("Area"),String("Router->SwitchAllocator"),String("Active")))->calculateSum();
            area += ((Result*)ms_model->parseQuery(String("Area"),String("Router->ClockTree"),String("Active")))->calculateSum();
            break;
        }
        case KITFOX_DSENT_REPEATED_LINK: {
            area = ((Result*)ms_model->parseQuery(String("Area"),String("RepeatedLink"),String("Active")))->calculateSum();
            break;
        }
        default: break;
    }
    
    return area;
}

bool energylib_dsent::update_library_variable(int Type, void *Value, bool IsLibraryVariable) {
    bool updated = false;
    
    if(IsLibraryVariable) return updated;
    
    switch(Type) {
        case KITFOX_DATA_CLOCK_FREQUENCY: {
            clock_frequency = *(double*)Value;
            ms_model->updateParameter(("Frequency"),ConvertString(clock_frequency));
            updated = true;
            break;
        }
        case KITFOX_DATA_TEMPERATURE: {
            double temperature = *(double*)Value;
            tech_model->remove(String("Temperature"));
            tech_model->set(String("Temperature"),ConvertString(temperature));
            updated = true;
            break;
        }
        case KITFOX_DATA_VOLTAGE: {
            double voltage = *(double*)Value;
            tech_model->remove(String("Vdd"));
            tech_model->set(String("Vdd"),ConvertString(voltage));
            updated = true;
            break;
        }
        default: {
            // Nothing to do
            return updated;
        }
    }
    
    delete std_cell_lib;
    StdCellLib* std_cell_lib = new StdCellLib(tech_model);
    tech_model->setStdCellLib(std_cell_lib);
    
    Model *old_model = ms_model;
    switch(energy_model) {
        case KITFOX_DSENT_ROUTER: {
            ms_model = ModelGen::createModel(String("Router"),String("Router"),tech_model);
            break;
        }   
        case KITFOX_DSENT_REPEATED_LINK: {
            ms_model = ModelGen::createModel(String("RepeatedLink"),String("RepeatedLink"),tech_model);
            break;
        }
        default: break;
    }

    ms_model->m_parameters_ = old_model->m_parameters_->clone();
    ms_model->m_properties_ = old_model->m_properties_->clone();
    delete old_model;
    
    ms_model->construct();
    ms_model->update();
    
    if(IsPerformTimingOptimization) {
        if(start_net_names.size() == 0) {
            ElectricalModel *electrical_model = (ElectricalModel*)ms_model;
            
            ElectricalTimingOptimizer timing_optimizer("Optimizer",electrical_model->getTechModel());
            timing_optimizer.setModel(electrical_model);
            timing_optimizer.construct();
            timing_optimizer.update();
            
            ElectricalTimingTree timing_tree(timing_optimizer.getInstanceName(),&timing_optimizer);
            
            const Map<PortInfo*> *input_ports = timing_optimizer.getInputs();
            Map<PortInfo*>::ConstIterator it_begin = input_ports->begin();
            Map<PortInfo*>::ConstIterator it_end = input_ports->end();
            Map<PortInfo*>::ConstIterator it;
            for(it = it_begin; it != it_end; it++) {
                const String &net_name = it->first;
                timing_tree.performTimingOpt(timing_optimizer.getNet(net_name,makeNetIndex(0)),1.0/clock_frequency);
            }
        }
        else {
            ElectricalModel *electrical_model = (ElectricalModel*)ms_model;
            ElectricalTimingTree timing_tree(electrical_model->getInstanceName(),electrical_model);
            for(unsigned i = 0; i < start_net_names.size(); i++) {
                const String &net_name = start_net_names[i];
                timing_tree.performTimingOpt(electrical_model->getNet(net_name),1.0/clock_frequency);
            }
        }
    }
    ms_model->evaluate();
    
    return updated;
}

String energylib_dsent::ConvertString(double v) {
    char buf[32];
    sprintf(buf,"%e",v);
    
    String str(buf);
    return str;
}

String energylib_dsent::ConvertString(int v) {
    char buf[32];
    sprintf(buf,"%d",v);
    
    String str(buf);
    return str;
}

