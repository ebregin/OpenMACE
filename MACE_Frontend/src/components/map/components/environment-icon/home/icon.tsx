import * as React from "react";

type Props = {
  width: number | string;
  height: number | string;
};

export default (props: Props) => {
  return (
    <svg
      width={props.width}
      height={props.height}
      viewBox="0 0 512 512"
      fill="none"
      xmlns="http://www.w3.org/2000/svg"
    >
      <path
        d="M256 0C153.755 0 70.573 83.182 70.573 185.426C70.573 312.314 236.512 498.593 243.577 506.461C250.213 513.852 261.799 513.839 268.423 506.461C275.488 498.593 441.427 312.314 441.427 185.426C441.425 83.182 358.244 0 256 0ZM256 278.719C204.558 278.719 162.708 236.868 162.708 185.426C162.708 133.984 204.559 92.134 256 92.134C307.441 92.134 349.291 133.985 349.291 185.427C349.291 236.869 307.441 278.719 256 278.719Z"
        fill="#38B2AC"
      />
      <circle cx="256" cy="185" r="133" fill="#4FD1C5" />
      <g clipPath="url(#clip0)">
        <path
          d="M341.927 152.583L321.876 136.542V101.591C321.876 98.534 319.402 96.0601 316.345 96.0601H294.231C291.179 96.0601 288.7 98.534 288.7 101.586L288.694 109.996L258.958 86.2073C256.937 84.5976 254.064 84.5976 252.044 86.2073L169.074 152.584C167.243 154.053 166.535 156.522 167.313 158.736C168.091 160.951 170.181 162.436 172.531 162.436H189.124V256.468C189.124 259.526 191.598 262 194.656 262H316.345C319.402 262 321.876 259.526 321.876 256.468V162.436H338.47C340.82 162.436 342.91 160.951 343.688 158.736C344.466 156.521 343.759 154.053 341.927 152.583V152.583ZM299.751 118.191L299.756 107.123H310.814V127.693L298.936 118.19C298.936 118.19 299.751 118.191 299.751 118.191V118.191ZM244.438 250.937V206.687H266.563V250.937H244.438ZM316.345 151.373C313.288 151.373 310.814 153.847 310.814 156.904V250.937H277.626V201.155C277.626 198.098 275.152 195.624 272.095 195.624H238.907C235.85 195.624 233.376 198.098 233.376 201.155V250.937H200.187V156.904C200.187 153.847 197.713 151.373 194.656 151.373H188.298L255.5 97.6106L322.703 151.374C322.703 151.373 316.345 151.373 316.345 151.373V151.373Z"
          fill="#B2F5EA"
        />
      </g>
      <defs>
        <clipPath id="clip0">
          <rect x="167" y="85" width="177" height="177" fill="white" />
        </clipPath>
      </defs>
    </svg>
  );
};
