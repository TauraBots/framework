/**
 * @module gamecontroller
 * Interface to the ssl gamecontroller protocol
 */

/**************************************************************************
*   Copyright 2018 Paul Bergmann                                          *
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
**************************************************************************/

const amunLocal = amun;

import { gameController } from "base/protobuf";
import * as World from "base/world";

type ConnectionState = "CONNECTED" | "UNCONNECTED";
let state: ConnectionState = "UNCONNECTED";

let message: gameController.ControllerToTeam | undefined = undefined;

/**
 * Check connection to the game controller and receive new messages
 */
export function _update() {
	message = undefined;
	if (World.TeamName === "ER-Force" && (World.OpponentTeamName !== "ER-Force" || World.TeamIsBlue)) {
		if (amunLocal.connectGameController()) {
			if (state === "UNCONNECTED") {
				state = "CONNECTED";
				amun.log("Connect to gameController");
				amunLocal.sendGameControllerMessage("TeamRegistration", { team_name: "ER-Force" });
			}

			message = amunLocal.getGameControllerMessage();
		} else {
			state = "UNCONNECTED";
		}
	}
}

/**
 * Check if a connection to the game controller is established
 * @returns true if connected to the game controller
 */
export function isConnected(): boolean {
	return state === "CONNECTED";
}

/**
 * Check if during this frame a message indicating a pushing foul was received.
 * This implies an advantage choice.
 * @returns the GameEvent if pushing occured, undefined otherwise
 */
export function getPushingEvent(): gameController.GameEvent.BotPushedBot | undefined {
	if (!message || !message.advantage_choice || message.advantage_choice.foul !== gameController.AdvantageChoice.Foul.PUSHING) {
		return undefined;
	}
	return message.advantage_choice.bot_pushed_bot;
}

/**
 * Check if during this frame a message indicating a collision foul was received.
 * This implies an advantage choice.
 * @returns the GameEvent if a collision occured, undefined otherwise
 */
export function getCollisionEvent(): gameController.GameEvent.BotCrashUnique | undefined {
	if (!message || !message.advantage_choice || message.advantage_choice.foul !== gameController.AdvantageChoice.Foul.COLLISION) {
		return undefined;
	}
	return message.advantage_choice.bot_crash_unique;
}

/**
 * Check if during this frame a message indicating a touch opponent in defense area foul was received.
 * This implies an advantage choice.
 * @returns the GameEvent if a touch opponent in defense area event occured, undefined otherwise
 */
export function getTouchOpponentInDefenseAreaEvent(): gameController.GameEvent.AttackerTouchedOpponentInDefenseArea | undefined {
	if (!message || !message.advantage_choice || message.advantage_choice.foul !== gameController.AdvantageChoice.Foul.ATTACKER_TOUCHED_OPPONENT_IN_DEFENSE_AREA) {
		return undefined;
	}
	return message.advantage_choice.attacker_touched_opponent_in_defense_area;
}

/**
 * Check if we are allowed to make an advantage choice
 * @returns true if we are allowed to make an advantage choice
 */
export function advantageFoulOccured(): boolean {
	return getCollisionEvent() != undefined || getPushingEvent() != undefined || getTouchOpponentInDefenseAreaEvent() != undefined;
}

/**
 * Request to change the keeper to the desired robot.
 * Will crash in debug while not in STOP
 * @param id - the id of the desired new keeper
 */
export function requestDesiredKeeper(id: number) {
	if (amunLocal.isDebug && World.RefereeState !== "Stop") {
		throw new Error("Trying to change keeper while not in STOP. The request would be rejected");
	}
	amunLocal.sendGameControllerMessage("TeamToController", { desired_keeper: id });
}

/**
 * Request a substitution at the next possibility
 */
export function requestSubstitution() {
	amunLocal.sendGameControllerMessage("TeamToController", { substitute_bot: true });
}
