
/* tslint:disable:prefer-method-signature */

/*
/// API for Ra. <br/>
// Amun offers serveral guarantees to the strategy: <br/>
// The values returned by getGeometry, getTeam, isBlue are guaranteed to remain constant for the whole strategy runtime.
// That is if any of the values changes the strategy is restarted! <br/>
// If coordinates are passed via the API these values are using <strong>global</strong> coordinates!
// This API may only be used by coded that provides a mapping between Amun and Strategy
module "amun"
*/

/**************************************************************************
*   Copyright 2015 Alexander Danzer, Michael Eischer, Philipp Nordhus     *
*   Robotics Erlangen e.V.                                                *
*   http://www.robotics-erlangen.de/                                      *
*   info@robotics-erlangen.de                                             *
*                                                                         *
*   This program is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation, either version 3 of the License, or     *
*   any later version.                                                    *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
*************************************************************************]]

--- Returns world state
-- @class function
-- @name getWorldState
-- @return protobuf.world.State - converted to lua table

--[[
separator for luadoc]]--

--- Returns world geometry
-- @class function
-- @name getGeometry
-- @return protobuf.world.Geometry - converted to lua table

--[[
separator for luadoc]]--

--- Returns team information
-- @class function
-- @name getTeam
-- @return protobuf.robot.Team - converted to lua table

--[[
separator for luadoc]]--

--- Query team color
-- @class function
-- @name isBlue
-- @return bool - true if this is the blue team, false otherwise

--[[
separator for luadoc]]--

--- Add a visualization
-- @class function
-- @name addVisualization
-- @param vis protobuf.amun.Visualization as table

--[[
separator for luadoc]]--

--- Add a circle
-- @class function
-- @name addVisualizationCircle
-- @param string name
-- @param number centerX
-- @param number centerY
-- @param number radius
-- @param number colorRed
-- @param number colorGreen
-- @param number colorBlue
-- @param number colorAlpha
-- @param bool isFilled
-- @param bool background
-- @param number linewidth

--[[
separator for luadoc]]--

--- Set commands for a robot
-- @class function
-- @name setCommand
-- @param int generation
-- @param int robotid
-- @param cmd protobuf.robot.StrategyCommand

--[[
separator for luadoc]]--

--- Log function.
-- If data is a string use ... as parameters for format.
-- Otherweise logs tostring(data)
-- @class function
-- @name log
-- @param data any - data to log
-- @param ... any - params for format (optional)

--[[
separator for luadoc]]--

--- Returns game state and referee information
-- @class function
-- @name getGameState
-- @return protobuf.GameState - converted to lua table

--[[
separator for luadoc]]--

--- Returns the user input
-- @class function
-- @name getUserInput
-- @return protobuf.UserInput - converted to lua table

--[[
separator for luadoc]]--

--- Returns current time
-- @class function
-- @name getCurrentTime
-- @return Number - time in nanoseconds (amun), seconds(strategy)

--[[
separator for luadoc]]--

--- Returns the absolute path to the folder containing the init script
-- @class function
-- @name getStrategyPath
-- @return String - path

--[[
separator for luadoc]]--

--- Returns list with names of enabled options
-- @class function
-- @name getSelectedOptions
-- @return String[] - options

--[[
separator for luadoc]]--

--- Sets a value in the debug tree
-- @class function
-- @name addDebug
-- @param key string
-- @param value number|bool|string|nil

--[[
separator for luadoc]]--

--- Add a value to the plotter
-- @class function
-- @name addPlot
-- @param name string
-- @param value number

--[[
separator for luadoc]]--

--- Set the exchange symbol for a robot
-- @class function
-- @name setRobotExchangeSymbol
-- @param generation number
-- @param id number
-- @param exchange bool

--[[
separator for luadoc]]--

--- Send arbitrary commands. Only works in debug mode
-- @class function
-- @name sendCommand
-- @param command amun.Command

--[[
separator for luadoc]]--

--- Send internal referee command. Only works in debug mode. Must be fully populated
-- @class function
-- @name sendRefereeCommand
-- @param command SSL_Referee

--[[
separator for luadoc]]--

--- Send mixed team info packet
-- @class function
-- @name sendMixedTeamInfo
-- @param data ssl::TeamPlan

--[[
separator for luadoc]]--

--- Send referee command over network. Only works in debug mode or as autoref. Must be fully populated
-- Only sends the data passed to the last call of this function during a strategy run.
-- The command_counter must be increased for every command change
-- @class function
-- @name sendNetworkRefereeCommand
-- @param command SSL_Referee

--[[
separator for luadoc]]--

--- Write output to debugger console
-- @class function
-- @name debuggerWrite
-- @param line string

--[[
separator for luadoc]]--

--- Wait for and read input from debugger console
-- @class function
-- @name debuggerRead
-- @return line string

--[[
separator for luadoc]]--

--- Check if performance mode is active
-- @class function
-- @name getPerformanceMode
-- @return mode boolean


--- Fetch the last referee remote control request reply
-- @class function
-- @name nextRefboxReply
-- @return reply table - the last reply or nil if none is available
*/

import * as pb from "base/protobuf";

interface AmunPublic {
	isDebug: boolean;
	strategyPath: string;
	isPerformanceMode: boolean;
	log(data: any, ...params: any[]): void;
	getCurrentTime(): number;
	setRobotExchangeSymbol(generation: number, id: number, exchange: boolean): void;
	nextRefboxReply(): pb.SSL_RefereeRemoteControlReply;

	// only in debug
	sendCommand(command: pb.amun.Command): void;
	sendNetworkRefereeCommand(command: pb.SSL_Referee): void;
}

interface Amun extends AmunPublic {
	getWorldState(): pb.world.State;
	getGeometry(): pb.world.Geometry;
	getTeam(): pb.robot.Team;
	isBlue(): boolean;
	addVisualization(vis: pb.amun.Visualization): void;
	addVisualizationCircle(name: string,
		centerX: number, centerY: number, radius: number,
		colorRed: number, colorGreen: number, colorBlue: number, colorAlpha: number,
		isFilled: boolean, background: boolean, linewidth: number
	): void;
	setCommand(generation: number, id: number, cmd: pb.robot.Command): void;
	getGameState(): pb.amun.GameState;
	getUserInput(): pb.amun.UserInput;
	getStrategyPath(): string;
	getSelectedOptions(): string[];
	addDebug(key: string, value?: number | boolean | string): void;
	addPlot(name: string, value: number): void;
	sendRefereeCommand(command: pb.SSL_Referee): void;
	sendMixedTeamInfo(data: pb.ssl.TeamPlan): void;
	debuggerWrite(line: string): void;
	debuggerRead(line: string): void;
	getPerformanceMode(): boolean;

	// undocumented
	luaRandomSetSeed(seed: number): void;
	luaRandom(): number;
	isReplay?: () => boolean;
}

declare global {
	let amun: Amun;
}

amun = {
	...amun,
	isDebug: false, // TODO
	isPerformanceMode: amun.getPerformanceMode!()
};

export function _hideFunctions() {
	let isDebug = amun.isDebug;
	let isPerformanceMode = amun.isPerformanceMode;
	let strategyPath = amun.getStrategyPath!();
	let getCurrentTime = amun.getCurrentTime;
	let setRobotExchangeSymbol = amun.setRobotExchangeSymbol;
	let nextRefboxReply = amun.nextRefboxReply;
	let log = amun.log;
	let sendCommand = amun.sendCommand;
	let sendNetworkRefereeCommand = amun.sendNetworkRefereeCommand;

	const DISABLED_FUNCTION = function(..._: any[]): any {
		throw new Error("Usage of disabled amun function");
	};

	amun = {
		isDebug: isDebug,
		isPerformanceMode: isPerformanceMode,
		strategyPath: strategyPath,
		getCurrentTime: function() {
			return getCurrentTime() * 1E-9;
		},
		setRobotExchangeSymbol: setRobotExchangeSymbol,
		nextRefboxReply: nextRefboxReply,
		log: log,
		sendCommand: isDebug ? sendCommand : DISABLED_FUNCTION,
		sendNetworkRefereeCommand: isDebug ? sendNetworkRefereeCommand : DISABLED_FUNCTION,

		getWorldState: DISABLED_FUNCTION,
		getGeometry: DISABLED_FUNCTION,
		getTeam: DISABLED_FUNCTION,
		isBlue: DISABLED_FUNCTION,
		addVisualization: DISABLED_FUNCTION,
		addVisualizationCircle: DISABLED_FUNCTION,
		setCommand: DISABLED_FUNCTION,
		getGameState: DISABLED_FUNCTION,
		getUserInput: DISABLED_FUNCTION,
		getStrategyPath: DISABLED_FUNCTION,
		getSelectedOptions: DISABLED_FUNCTION,
		addDebug: DISABLED_FUNCTION,
		addPlot: DISABLED_FUNCTION,
		sendRefereeCommand: DISABLED_FUNCTION,
		sendMixedTeamInfo: DISABLED_FUNCTION,
		debuggerWrite: DISABLED_FUNCTION,
		debuggerRead: DISABLED_FUNCTION,
		getPerformanceMode: DISABLED_FUNCTION,

		luaRandomSetSeed: DISABLED_FUNCTION,
		luaRandom: DISABLED_FUNCTION
	};
}

export const log = amun.log;
