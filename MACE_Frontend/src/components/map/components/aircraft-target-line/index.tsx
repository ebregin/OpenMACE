import * as React from "react";
import { Marker, Tooltip, Polyline } from "react-leaflet";
import * as L from "leaflet";
import { renderToString } from "react-dom/server";
import AppContext, { Context } from "../../../../Context";
import colors from "../../../../util/colors";
const { useContext } = React;
import { getTailNumberFromAgentID, getColorForIndex } from "../../../../util/helpers";
import { MdInvertColors } from "react-icons/md";


type Props = {
  data: Aircraft.TargetPayload;
};

const TargetLine = (props: Props) => {
  const { aircrafts } = useContext<Context>(AppContext);
  const aircraft = aircrafts.find((a) => a.agentID === props.data.agentID);
  return (
    <Polyline
      positions={[props.data.location, aircraft ? aircraft.location : props.data.location]}
      color={aircraft.selected ? aircraft.color[500] : aircraft.color[500] + "30"}
      weight={5}
    />

  );
};


export default TargetLine;