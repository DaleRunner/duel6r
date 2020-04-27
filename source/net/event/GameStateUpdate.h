/*
 * GameStateUpdate.h
 *
 *  Created on: Mar 30, 2020
 *      Author: fanick
 */

#ifndef SOURCE_NET_EVENT_GAMESTATEUPDATE_H_
#define SOURCE_NET_EVENT_GAMESTATEUPDATE_H_

#include "../Event.h"
#include "../object/Player.h"

namespace Duel6::net {
    class GameStateUpdate : public Event<GameStateUpdate, EventType::GAME_STATE_UPDATE>{
    public:
        tick_t inputTick;
        tick_t confirmInputTick;
        tick_t snapshotTick;
        bool hasWinner = false;
        std::vector<Player> players;
        template<typename Stream>
        bool serialize(Stream &s) {
            return s & inputTick
                && s & confirmInputTick
                && s & snapshotTick
                && s & players
                && s & hasWinner;
        }
    };
}



#endif /* SOURCE_NET_EVENT_GAMESTATEUPDATE_H_ */
