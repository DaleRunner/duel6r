/*
* Copyright (c) 2006, Ondrej Danek (www.ondrej-danek.net)
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Ondrej Danek nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "math/Math.h"
#include "Sound.h"
#include "BonusList.h"
#include "World.h"
#include "collision/Collision.h"

namespace Duel6 {
    BonusList::BonusList(const GameSettings &settings, const GameResources &resources, World &world)
            : settings(settings), texture(resources.getBonusTextures()), world(world) {}

    void BonusList::render(Renderer &renderer) const {
        for (const Bonus &bonus : bonuses) {
            bonus.render(renderer, texture);
        }

        for (const LyingWeapon &weapon : weapons) {
            weapon.render(renderer);
        }
    }

    void BonusList::addRandomBonus() {
        bool weapon = (Math::random(2) == 1);
        const ValidPositionList& validPositionList = findValidPositions(weapon);

        if (validPositionList.empty()) {
            return;
        }

        ValidPosition position = validPositionList[Math::random(validPositionList.size())];
        Int32 x = position.first, y = position.second;

        bool isOverLimit = (weapons.size() + bonuses.size()) >= (validPositionList.size() / 4);

        if (weapon) {
            Int32 bullets = Math::random(10) + 10;
            weapons.push_back(LyingWeapon(Weapon::getRandomEnabled(settings), bullets, Vector(x, y)));
            if(isOverLimit) {
                weapons.pop_front();
            }
        } else {
            const BonusType *type = BonusType::ALL[Math::random(BonusType::ALL.size())];
            bool random = Math::random(RANDOM_BONUS_FREQUENCY) == 0;
            Int32 duration = type->isOneTime() ? 0 : 13 + Math::random(17);
            bonuses.push_back(
                    Bonus(type, duration, Vector(x + 0.2f, y + 0.2f), random ? 0 : type->getTextureIndex()));
            if(isOverLimit) {
                bonuses.pop_front();
            }
        }
    }

    BonusList::ValidPositionList BonusList::findValidPositions(bool weapon) {
        const Level &level = world.getLevel();

        std::vector<ValidPosition> positions;
        positions.reserve(level.getHeight() * level.getWidth());
        for (Int32 y = 0; y < level.getHeight(); y++) {
            for (Int32 x = 0; x < level.getWidth(); x++) {
                if (isValidPosition(x, y, weapon)) {
                    positions.push_back(std::make_pair(x, y));
                }
            }
        }
        return positions;
    }

    bool BonusList::isValidPosition(const Int32 x, const Int32 y, bool weapon) {
        const Level &level = world.getLevel();

        if(level.isRaisingWater() && level.getWaterLevel() >= y) {
            return false;
        }

        if (!level.isEmpty(x, y) || (weapon && !level.isWall(x, y - 1, true))) {
            return false;
        }

        Rectangle candidate = Rectangle::fromCorners(Vector(x - 2, y - 2), Vector(x + 2, y + 2));
        for (Player &player : world.getPlayers()) {
            if (Collision::rectangles(candidate, player.getCollisionRect())) {
                return false;
            }
        }

        candidate = Rectangle::fromCorners(Vector(x, y), Vector(x + 1, y + 1));
        for (const Bonus &bonus : bonuses) {
            if (Collision::rectangles(candidate, bonus.getCollisionRect())) {
                return false;
            }
        }

        for (const LyingWeapon &lyingWeapon : weapons) {
            if (Collision::rectangles(candidate, lyingWeapon.getCollisionRect())) {
                return false;
            }
        }

        return true;
    }

    void BonusList::addPlayerGun(Player &player, const CollidingEntity &playerCollider) {
        weapons.push_back(LyingWeapon(player.getWeapon(), player.getAmmo(), player.getReloadTime(), playerCollider));
    }

    void BonusList::checkBonus(Player &player) {
        auto bonusIter = bonuses.begin();
        while (bonusIter != bonuses.end()) {
            Bonus &bonus = *bonusIter;
            const BonusType *type = bonusIter->getType();

            bool collides = Collision::rectangles(bonus.getCollisionRect(), player.getCollisionRect());
            if (collides && type->isApplicable(player, world)) {
                if (type->isOneTime()) {
                    type->onApply(player, world, 0);
                } else {
                    player.setBonus(type, bonus.getDuration());
                }

                player.playSound(PlayerSounds::Type::PickedBonus);
                bonusIter = bonuses.erase(bonusIter);
            } else {
                ++bonusIter;
            }
        }
    }

    void BonusList::update(Float32 elapsedTime) {
        for(auto & weapon : weapons){
            weapon.collider.collideWithElevators(world.getElevatorList(), elapsedTime);
            weapon.collider.collideWithLevel(world.getLevel(), elapsedTime, elapsedTime);
            if(weapon.pickTimeout > 0){
                weapon.pickTimeout -= elapsedTime;
            }
        }
    }

    void BonusList::checkWeapon(Player &player) {
        auto weaponIter = weapons.begin();
        while (weaponIter != weapons.end()) {
            LyingWeapon &weapon = *weaponIter;
            Weapon type = weapon.getWeapon();

            bool collides = Collision::rectangles(weapon.getCollisionRect(), player.getCollisionRect());
            if (weapon.pickTimeout <= 0.0f && collides) {
                if (player.hasGun() && player.getAmmo() > 0) {
                    // Leave the current weapon at the same place
                    addPlayerGun(player, player.getCollider());
                }

                player.pickWeapon(type, weapon.getBullets(), weapon.remainingReloadTime);
                world.getMessageQueue().add(player, Format("You picked up gun {0}") << type.getName());

                weapons.erase(weaponIter);
                return;
            } else {
                ++weaponIter;
            }
        }
    }
}
