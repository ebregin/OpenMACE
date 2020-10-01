import * as React from "react";
import styles from "./styles";
import {
    getFields,
    getDisplayParam,
    getStringValue,
    getBatteryTextColor,
    getTextSeverityColor,
    getShowButton
} from "../../../util/helpers";
import ReactTooltip from "react-tooltip";
import { FaBatteryEmpty, FaBatteryQuarter, FaBatteryHalf, FaBatteryThreeQuarters, FaBatteryFull, FaLevelUpAlt, FaCaretDown } from "react-icons/fa";
import { FiPlay, FiPause, FiHome, FiCrosshair, FiX, FiTarget, FiCheck } from "react-icons/fi";
import { MdFlightTakeoff, MdFlightLand, MdBubbleChart } from "react-icons/md";
import colors from "../../../util/colors";
import { LatLng } from "leaflet";
import {
    AttitudeIndicator,
    HeadingIndicator
} from 'react-flight-indicators'
import Select from "react-select";
import { Vertex } from "../../../data-types";

const hiddenFields = ["lat", "lng"];

const ROTOR_MODE_OPTIONS = [
    { value: "UNKNOWN", label: "Unknown" },
    { value: "AUTO", label: "Auto" },
    { value: "BRAKE", label: "Brake" },
    { value: "GUIDED", label: "Guided" },
    { value: "LAND", label: "Land" },
    { value: "LOITER", label: "Loiter" },
    { value: "POSHOLD", label: "Position hold" },
    { value: "RTL", label: "Return to Launch" },
    { value: "STABILIZE", label: "Stabilize" }
  ];

const FIXED_WING_MODE_OPTIONS = [
    { value: "UNKNOWN", label: "Unknown" },
    { value: "AUTO", label: "Auto" },
    { value: "CIRCLE", label: "Circle" },
    { value: "CRUISE", label: "Cruise" },
    { value: "GUIDED", label: "Guided" },
    { value: "INITIALISING", label: "Initialising" },
    { value: "LOITER", label: "Loiter" },
    { value: "MANUAL", label: "Manual" },
    { value: "RTL", label: "Return to launch" },
    { value: "STABILIZE", label: "Stabilize" },
    { value: "TAKEOFF", label: "Takeoff" }
    // { value: "QSTABILIZE", label: "QStabilize" },
    // { value: "QHOVER", label: "QHover" },
    // { value: "QLOITER", label: "QLoiter" },
    // { value: "QLAND", label: "QLand" },
    // { value: "QRTL", label: "QReturn to Launch" },
    // { value: "NR", label: "NR" },
  ];

type Props = {
    data: Aircraft.AircraftPayload;
    onRequestCenter: (LatLng) => void;
    onCommand: (command: string, filteredAircrafts: Aircraft.AircraftPayload[], payload: string[]) => void;
    onUpdateGoHerePts: (point: Vertex) => void;
    toggleGoHerePt: (show: boolean) => void;
    target: Vertex;
    defaultAltitude: number;
    onToggleSelect: (agentID: string[]) => void;
};

/**
 *  1. Make cross hair center on plane
 *  2. Transform the keys to readable
 *  3. Add ability to toggle
 */

/**
 *  Check to see if any props have changed other than location. Location is not displayed on this UI so no need to re-render if it changes
 */
const excludedKeys = ["location"];

const arePropsSame = (prevProps: Props, nextProps: Props) => {
    let same = true;
    const keysToCheck = Object.keys(nextProps.data).filter(
        (k) => excludedKeys.indexOf(k) === -1
    );
    const { data: prevData } = prevProps;
    const { data: nextData } = nextProps;
    keysToCheck.forEach((key) => {
        if (JSON.stringify(prevData[key]) !== JSON.stringify(nextData[key])) {
            same = false;
            return same;
        }
    });
    return same;
};

export default React.memo((props: Props) => {
    const fields = getFields(props.data, ["agentID"]);
    const [altitude, setAltitude] = React.useState(props.defaultAltitude);
    const MODE_OPTIONS = fields["vehicle_type"] === "QUADROTOR" ? ROTOR_MODE_OPTIONS : FIXED_WING_MODE_OPTIONS;
    const ICON_COLOR = colors.gray[700];
    const DISABLED_ICON_COLOR = colors.gray[400];
    
    const requestCenter = () => {
        const { location } = props.data;
        props.onRequestCenter(location);
    };
    
    const updateMission = () => {
        let command: string = "FORCE_DATA_SYNC";
        let payload = [];
        props.onCommand(command, [props.data], payload);
    }

    const arm = () => {
        let command: string = "SET_VEHICLE_ARM";
        let payload = {
            arm: true
        };
        props.onCommand(command, [props.data], [JSON.stringify(payload)]);
    }

    const takeoff = () => {
        let command: string = "TAKEOFF";
        let payload = {
            takeoffPosition: {
                alt: altitude
            },
            latLonFlag: false
        };
        props.onCommand(command, [props.data], [JSON.stringify(payload)]);
        updateMission();
    };
    const land = () => {
        let command: string = "LAND";
        let payload = [];
        props.onCommand(command, [props.data], payload);
    };
    const startMission = () => {
        updateMission();
        let command: string = "START_MISSION";
        let payload = [];
        props.onCommand(command, [props.data], payload);
    };
    const pauseMission = () => {
        let command: string = "PAUSE_MISSION";
        let payload = [];
        props.onCommand(command, [props.data], payload);
    };
    const returnToLaunch = () => {
        let command: string = "RTL";
        let payload = [];
        props.onCommand(command, [props.data], payload);
    };
    const goHere = () => {
        props.toggleGoHerePt(false);
        ReactTooltip.hide();
        let command: string = "SET_GO_HERE";
        props.onCommand(command, [props.data], [JSON.stringify(props.target)]);
    }

    const getBatteryIcon = () => {
        if(fields["battery_remaining"] >= 90) {
            return <FaBatteryFull style={styles.rotatedIcon} color={colors.green[600]} size={20} />
        }
        else if(fields["battery_remaining"] >= 75 && fields["battery_remaining"] < 90) {
            return <FaBatteryThreeQuarters style={styles.rotatedIcon} color={colors.green[600]} size={20} />
        }
        else if(fields["battery_remaining"] < 75 && fields["battery_remaining"] >= 50) {
            return <FaBatteryHalf style={styles.rotatedIcon} color={colors.yellow[600]} size={20} />
        }
        else if(fields["battery_remaining"] < 50 && fields["battery_remaining"] >= 25) {
            return <FaBatteryQuarter style={styles.rotatedIcon} color={colors.orange[600]} size={20} />
        }
        else {
            return <FaBatteryEmpty style={styles.rotatedIcon} color={colors.red[600]} size={20} />
        }
    }

    const getAircraftText = () => {
        let textColor = colors.gray[500];
        let text = "No messages"

        if(fields["textStr"]) {
            text = fields["textStr"];
            textColor = getTextSeverityColor(fields["textSeverity"])
        }

        return <span style={styles.label && {color: textColor}}>{text}</span>
    }

    const setMode = (selectedMode: string) => {
        let command: string = "SET_VEHICLE_MODE";
        let payload = {
            mode: selectedMode["value"]
        };
        props.onCommand(command, [props.data], [JSON.stringify(payload)]);
    }

    const getValueFromMode = () => {
        // console.log(fields["mode"]);
        for (let i = 0; i < MODE_OPTIONS.length; i++) {
            if (fields["mode"] === MODE_OPTIONS[i]["value"]) {
                return MODE_OPTIONS[i];
            }
        }

        return MODE_OPTIONS[0];
    }

    const updateAltitude = (e) => {
        const { name, value } = e.target;
        setAltitude(parseFloat(value));
    };

    const updateTarget = (field: string, value: number) => {
        let newTarget = props.target;
        if (field === "lat") { newTarget["lat"] = value; }
        if (field === "lng") { newTarget["lng"] = value; }
        if (field === "alt") { newTarget["alt"] = value; }
        props.onUpdateGoHerePts(newTarget);

    }

    const setTargetToCurrentLocation = (e) => {
        props.toggleGoHerePt(true);
        props.onUpdateGoHerePts(props.data.location);
    }
    
    const getVehicleIcon = () => {
        if(fields["vehicle_type"] === "FIXED_WING") {
            return <img src='./icons/fixed-wing.png' style={{height: 20, width: 20}} alt={fields["vehicle_type"]} />
        }
        else {
            return <img src='./icons/drone-icon.png' style={{height: 25, width: 25}} alt={fields["vehicle_type"]} />
        }
    }

    const getVehicleColor = () => {
        return props.data.color[500];
    }

    const selectAircraft = (event: React.MouseEvent<HTMLSpanElement, MouseEvent>) => {
        props.onToggleSelect([props.data.agentID]);
    }

    const showButton = () => {
        return getShowButton(props.data.vehicle_state, props.data.vehicle_type);
    }


    return (
        <div style={styles.container}>
            <div onClick={(e: React.MouseEvent<HTMLSpanElement, MouseEvent>) => selectAircraft(e)} style={{...styles.header, backgroundColor: getVehicleColor()}}>
                <span style={styles.title}>{props.data.agentID}</span>
                {getVehicleIcon()}
                <button style={styles.centerButton} onClick={arm}>
                    <FiCheck color={colors.teal[100]} size={20} />
                </button>
                <button style={styles.centerButton} onClick={requestCenter}>
                    <FiCrosshair color={colors.teal[100]} size={20} />
                </button>
            </div>


            <div
                key={"mode"}
                style={Object.assign(
                    {},
                    styles.selectRow
                )}
            >
                <Select
                    options={MODE_OPTIONS}
                    value={getValueFromMode()}
                    onChange={(ev) => {
                        setMode(ev);
                    }}
                />
            </div>

            <div style={styles.hudData}>
                <div
                    key={"behavior"}
                    style={Object.assign(
                        {},
                        styles.hudRow
                    )}
                >
                    <div style={styles.hudElement}>
                        <FaLevelUpAlt color={ICON_COLOR} size={20} />
                        <span style={styles.hudValue}>{getStringValue(fields["alt"].toFixed(2))}</span>
                    </div>
                    <div style={styles.hudElement}>
                        {getBatteryIcon()}
                        <span style={styles.hudValue && {color: getBatteryTextColor(fields["battery_remaining"])}}>{getStringValue(fields["battery_remaining"]) + "%"}</span>
                    </div>
                    <div style={styles.hudElement}>
                        <MdBubbleChart color={ICON_COLOR} size={20} />
                        <span style={styles.hudValue}>{getStringValue(fields["behavior_state"])}</span>
                    </div>
                </div>
            </div>
            

            <div style={styles.indicatorContainer}>                
                <HeadingIndicator heading={fields["yaw"]} showBox={false} />

                {/* TODO-PAT/AARON: Is negating the roll angle correct here? The roll angle is reported as + for right roll from MACE, this indicator seems to plot the opposite */}
                <AttitudeIndicator roll={-fields["roll"]} pitch={fields["pitch"]} showBox={false} />
            </div>

            <div style={styles.commandsContainer}>
                    <div style={styles.commandButtons}>
                        { showButton().takeoff.show &&
                        <button style={styles.centerButton}>
                            <MdFlightTakeoff data-for={props.data.agentID + "_tooltip"} data-tip='custom show' data-event='click' color={ICON_COLOR} size={20} />        
                        </button>
                        }
                        { showButton().takeoff.show &&
                        <ReactTooltip id={props.data.agentID + "_tooltip"} place='left' globalEventOff='click' clickable={true} backgroundColor={colors.white}>
                            <div style={styles.singleSettingContainer}>
                                <label style={styles.inputLabel}>Takeoff Alt (m):</label>
                                <input
                                    id="takeoff-tooltip-input"
                                    type="number"
                                    min={0}
                                    defaultValue={altitude}
                                    onChange={updateAltitude}
                                    name={"alt"}
                                    style={styles.input}
                                />
                            </div>
                            <div style={styles.tooltipButtons}>
                                <button style={styles.centerButton}>
                                    <FiX color={ICON_COLOR} size={20} />                        
                                </button>
                                <button style={styles.centerButton} onClick={takeoff}>
                                    <MdFlightTakeoff color={ICON_COLOR} size={20} />                        
                                </button>
                            </div>
                        </ReactTooltip>
                        }
                        { showButton().land.show &&
                        <button style={showButton().land.disabled ? styles.disabled_centerButton : styles.centerButton} disabled={showButton().land.disabled} onClick={land}>
                            { showButton().land.disabled ? 
                                <MdFlightLand color={DISABLED_ICON_COLOR} size={20} />
                                :
                                <MdFlightLand color={ICON_COLOR} size={20} />
                            }
                        </button>
                        }  
                        { showButton().startmission.show &&                    
                        <button style={showButton().startmission.disabled ? styles.disabled_centerButton : styles.centerButton} disabled={showButton().startmission.disabled}>
                            { showButton().startmission.disabled ? 
                                <FiPlay color={DISABLED_ICON_COLOR} size={20} />
                            :
                                <FiPlay color={ICON_COLOR} size={20} />
                            }
                        </button>
                        }
                        { showButton().pausemission.show &&
                        <button style={showButton().pausemission.disabled ? styles.disabled_centerButton : styles.centerButton} disabled={showButton().pausemission.disabled} onClick={pauseMission}>
                            { showButton().pausemission.disabled ? 
                                <FiPause color={DISABLED_ICON_COLOR} size={20} />
                            :
                                <FiPause color={ICON_COLOR} size={20} />
                            }
                        </button>
                        }
                        {showButton().rtl.show &&
                        <button style={showButton().rtl.disabled ? styles.disabled_centerButton : styles.centerButton} disabled={showButton().rtl.disabled} onClick={returnToLaunch}>
                            { showButton().rtl.disabled ?
                                <FiHome color={DISABLED_ICON_COLOR} size={20} />
                            :
                                <FiHome color={ICON_COLOR} size={20} />
                            }
                        </button>
                        }

                        {showButton().setgohere.show &&
                            <button style={showButton().setgohere.disabled ? styles.disabled_centerButton : styles.centerButton} disabled={showButton().setgohere.disabled}>
                                { showButton().setgohere.disabled ?
                                    <FiTarget color={DISABLED_ICON_COLOR} size={20} />
                                :
                                    <FiTarget data-for={"gohere_" + props.data.agentID + "_tooltip"} data-tip='custom show' data-event='click' color={ICON_COLOR} size={20} />
                                }
                            </button>
                        }
                        {(showButton().setgohere.show && !showButton().setgohere.disabled) &&
                            <ReactTooltip id={"gohere_" + props.data.agentID + "_tooltip"} afterShow={setTargetToCurrentLocation} place='left' clickable={true} backgroundColor={colors.white}>
                                <div style={styles.singleSettingContainer}>
                                    <label style={styles.inputLabel}>Latitude:</label>
                                    <input
                                        id="latitude-tooltip-input"
                                        type="number"
                                        value={props.target.lat}
                                        onChange={(e) => {
                                            const { name, value } = e.target;
                                            updateTarget(name,parseFloat(value));
                                        }}
                                        name={"lat"}
                                        style={styles.input}
                                    />
                                    <label style={styles.inputLabel}>Longitude:</label>
                                    <input
                                        id="longitude-tooltip-input"
                                        type="number"
                                        value={props.target.lng}
                                        onChange={(e) => {
                                            const { name, value } = e.target;
                                            updateTarget(name,parseFloat(value));
                                        }}
                                        name={"lng"}
                                        style={styles.input}
                                    />
                                    <label style={styles.inputLabel}>Altitude (m):</label>
                                    <input
                                        id="altitude-tooltip-input"
                                        type="number"
                                        min={altitude}
                                        value={props.target.alt}
                                        onChange={(e) => {
                                            const { name, value } = e.target;
                                            updateTarget(name,parseFloat(value));
                                        }}
                                        name={"alt"}
                                        style={styles.input}
                                    />
                                </div>
                                <div style={styles.tooltipButtons}>
                                    <button style={styles.centerButton} onClick={() => { ReactTooltip.hide(); props.toggleGoHerePt(false);}}>
                                        <FiX color={ICON_COLOR} size={20} />                        
                                    </button>
                                    <button style={styles.centerButton} onClick={goHere}>
                                        <FiCheck color={ICON_COLOR} size={20} />                        
                                    </button>
                                </div>
                            </ReactTooltip>
                        }

                    </div>
                
                
            </div>


            <div style={styles.row}>
                {getAircraftText()}
            </div>


        </div>
    );
}, arePropsSame);