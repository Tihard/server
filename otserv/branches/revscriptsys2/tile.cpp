//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// a Tile represents a single field on the map.
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////
#include "otpch.h"

#include "tile.h"
#include "housetile.h"
#include "game.h"
#include "combat.h"
#include "actor.h"
#include "player.h"
#include "teleport.h"
#include "trashholder.h"
#include "mailbox.h"

extern Game g_game;

StaticTile real_null_tile(0xFFFF, 0xFFFF, 0xFFFF);
Tile& Tile::null_tile = real_null_tile;

HouseTile* Tile::getHouseTile()
{
	if(isHouseTile())
		return static_cast<HouseTile*>(this);
	return NULL;
}

const HouseTile* Tile::getHouseTile() const
{
	if(isHouseTile())
		return static_cast<const HouseTile*>(this);
	return NULL;
}

bool Tile::isHouseTile() const
{
	return hasFlag(TILEPROP_HOUSE);
}

bool Tile::hasHeight(uint32_t n) const
{
	uint32_t height = 0;

	if(ground){
		if(ground->hasHeight()){
			++height;
		}

		if(n == height){
			return true;
		}
	}

	if(const TileItemVector* items = getItemList()){
		for(ItemVector::const_iterator it = items->begin(); it != items->end(); ++it){
			if((*it)->hasHeight()){
				++height;
			}

			if(n == height){
				return true;
			}
		}
	}

	return false;
}

uint32_t Tile::getCreatureCount() const
{
	if(const CreatureVector* creatures = getCreatures())
		return creatures->size();
	return 0;
}

uint32_t Tile::getItemCount() const
{
	if(const TileItemVector* items = getItemList()){
		return (uint32_t)items->size();
	}

	return 0;
}

uint32_t Tile::getTopItemCount() const
{
	if(const TileItemVector* items = getItemList()){
		return items->getTopItemCount();
	}

	return 0;
}

uint32_t Tile::getDownItemCount() const
{
	if(const TileItemVector* items =getItemList()){
		return items->getDownItemCount();
	}

	return 0;
}

Teleport* Tile::getTeleportItem() const
{
	if(!hasFlag(TILEPROP_TELEPORT)){
		return NULL;
	}

	if(const TileItemVector* items = getItemList()){
		for(ItemVector::const_reverse_iterator it = items->rbegin(); it != items->rend(); ++it){
			if((*it)->getTeleport())
				return (*it)->getTeleport();
		}
	}

	return NULL;
}

MagicField* Tile::getFieldItem() const
{
	if(!hasFlag(TILEPROP_MAGICFIELD)){
		return NULL;
	}

	if(ground && ground->getMagicField()){
		return ground->getMagicField();
	}

	if(const TileItemVector* items = getItemList()){
		for(ItemVector::const_reverse_iterator it = items->rbegin(); it != items->rend(); ++it){
			if((*it)->getMagicField())
				return (*it)->getMagicField();
		}
	}

	return NULL;
}

TrashHolder* Tile::getTrashHolder() const
{
	if(!hasFlag(TILEPROP_TRASHHOLDER)){
		return NULL;
	}

	if(ground && ground->getTrashHolder()){
		return ground->getTrashHolder();
	}

	if(const TileItemVector* items = getItemList()){
		for(ItemVector::const_reverse_iterator it = items->rbegin(); it != items->rend(); ++it){
			if((*it)->getTrashHolder())
				return (*it)->getTrashHolder();
		}
	}

	return NULL;
}

Mailbox* Tile::getMailbox() const
{
	if(!hasFlag(TILEPROP_MAILBOX)){
		return NULL;
	}

	if(ground && ground->getMailbox()){
		return ground->getMailbox();
	}

	if(const TileItemVector* items = getItemList()){
		for(ItemVector::const_reverse_iterator it = items->rbegin(); it != items->rend(); ++it){
			if((*it)->getMailbox())
				return (*it)->getMailbox();
		}
	}

	return NULL;
}

BedItem* Tile::getBedItem() const
{
	if(!hasFlag(TILEPROP_BED)){
		return NULL;
	}

	if(ground && ground->getBed()){
		return ground->getBed();
	}

	if(const TileItemVector* items = getItemList()){
		for(ItemVector::const_reverse_iterator it = items->rbegin(); it != items->rend(); ++it){
			if((*it)->getBed())
				return (*it)->getBed();
		}
	}

	return NULL;
}

Creature* Tile::getTopCreature()
{
	if(CreatureVector* creatures = getCreatures()){
		if(!creatures->empty()){
			return *creatures->begin();
		}
	}

	return NULL;
}

Item* Tile::getTopDownItem()
{
	if(TileItemVector* items = getItemList()){
		if(items->getDownItemCount() > 0){
			return *items->getBeginDownItem();
		}
	}

	return NULL;
}

Item* Tile::getTopTopItem()
{
	if(TileItemVector* items = getItemList()){
		if(items->getTopItemCount() > 0){
			return *(items->getEndTopItem() - 1);
		}
	}

	return NULL;
}

Item* Tile::getItemByTopOrder(uint32_t topOrder)
{
	//topOrder:
	//1: borders
	//2: ladders, signs, splashes
	//3: doors etc
	//4: creatures

	if(TileItemVector* items = getItemList()){
		ItemVector::reverse_iterator itEnd = ItemVector::reverse_iterator(items->getBeginTopItem());
		for(ItemVector::reverse_iterator it = ItemVector::reverse_iterator(items->getEndTopItem()); it != itEnd; ++it){
			if(Item::items[(*it)->getID()].alwaysOnTopOrder == (int32_t)topOrder){
				return (*it);
			}
		}
	}

	return NULL;
}

Thing* Tile::getTopVisibleThing(const Creature* creature)
{
	if(const CreatureVector* creatures = getCreatures()){
		for(CreatureVector::const_iterator cit = creatures->begin(); cit != creatures->end(); ++cit){
			if(creature->canSeeCreature(*cit)){
				return (*cit);
			}
		}
	}

	TileItemVector* items = getItemList();

	if(items){
		for(ItemVector::iterator it = items->getBeginDownItem(); it != items->getEndDownItem(); ++it){
			const ItemType& iit = Item::items[(*it)->getID()];
			if(!iit.lookThrough){
				return (*it);
			}
		}

		ItemVector::reverse_iterator itEnd = ItemVector::reverse_iterator(items->getBeginTopItem());
		for(ItemVector::reverse_iterator it = ItemVector::reverse_iterator(items->getEndTopItem()); it != itEnd; ++it){
			const ItemType& iit = Item::items[(*it)->getID()];
			if(!iit.lookThrough){
				return (*it);
			}
		}
	}

	if(ground)
		return ground;

	return NULL;
}

Creature* Tile::getTopVisibleCreature(const Creature* creature)
{
	if(CreatureVector* creatures = getCreatures()){
		for(CreatureVector::const_iterator cit = creatures->begin(); cit != creatures->end(); ++cit){
			if((*cit)->getPlayer() && (*cit)->getPlayer()->hasFlag(PlayerFlag_CannotBeSeen))
				continue;

			return (*cit);
		}
	}

	return NULL;
}

const Creature* Tile::getTopVisibleCreature(const Creature* creature) const
{
	if(const CreatureVector* creatures = getCreatures()){
		for(CreatureVector::const_iterator cit = creatures->begin(); cit != creatures->end(); ++cit){
			if((*cit)->getPlayer() && (*cit)->getPlayer()->hasFlag(PlayerFlag_CannotBeSeen))
				continue;

			return (*cit);
		}
	}

	return NULL;
}

void Tile::onAddTileItem(Item* item)
{
	updateTileFlags(item, false);

	const Position& cylinderMapPos = getPosition();

	const SpectatorVec& list = g_game.getSpectators(cylinderMapPos);
	SpectatorVec::const_iterator it;

	//send to client
	Player* tmpPlayer = NULL;
	for(it = list.begin(); it != list.end(); ++it){
		if((tmpPlayer = (*it)->getPlayer())){
			tmpPlayer->sendAddTileItem(this, cylinderMapPos, item);
		}
	}

	//event methods
	for(it = list.begin(); it != list.end(); ++it){
		(*it)->onAddTileItem(this, cylinderMapPos, item);
	}
}

void Tile::onUpdateTileItem(Item* oldItem, const ItemType& oldType, Item* newItem, const ItemType& newType)
{
	const Position& cylinderMapPos = getPosition();

	const SpectatorVec& list = g_game.getSpectators(cylinderMapPos);
	SpectatorVec::const_iterator it;

	//send to client
	Player* tmpPlayer = NULL;
	for(it = list.begin(); it != list.end(); ++it){
		if((tmpPlayer = (*it)->getPlayer())){
			tmpPlayer->sendUpdateTileItem(this, cylinderMapPos, oldItem, newItem);
		}
	}

	//event methods
	for(it = list.begin(); it != list.end(); ++it){
		(*it)->onUpdateTileItem(this, cylinderMapPos, oldItem, oldType, newItem, newType);
	}
}

void Tile::onRemoveTileItem(const SpectatorVec& list, std::vector<uint32_t>& oldStackPosVector, Item* item)
{
	updateTileFlags(item, true);

	const Position& cylinderMapPos = getPosition();
	const ItemType& iType = Item::items[item->getID()];

	SpectatorVec::const_iterator it;

	//send to client
	Player* tmpPlayer = NULL;
	uint32_t i = 0;
	for(it = list.begin(); it != list.end(); ++it){
		if((tmpPlayer = (*it)->getPlayer())){
			tmpPlayer->sendRemoveTileItem(this, cylinderMapPos, oldStackPosVector[i], item);
			++i;
		}
	}

	//event methods
	for(it = list.begin(); it != list.end(); ++it){
		(*it)->onRemoveTileItem(this, cylinderMapPos, iType, item);
	}
}

void Tile::onUpdateTile()
{
	const Position& cylinderMapPos = getPosition();

	const SpectatorVec& list = g_game.getSpectators(cylinderMapPos);
	SpectatorVec::const_iterator it;

	//send to client
	Player* tmpPlayer = NULL;
	for(it = list.begin(); it != list.end(); ++it){
		if((tmpPlayer = (*it)->getPlayer())){
			tmpPlayer->sendUpdateTile(this, cylinderMapPos);
		}
	}

	//event methods
	for(it = list.begin(); it != list.end(); ++it){
		(*it)->onUpdateTile(this, cylinderMapPos);
	}
}

void Tile::moveCreature(Creature* actor, Creature* creature, Cylinder* toCylinder, bool teleport /* = false*/)
{
	Tile* newTile = toCylinder->getTile();
	int32_t oldStackPos = __getIndexOfThing(creature);

	Position oldPos = getPosition();
	Position newPos = newTile->getPosition();

	Player* tmpPlayer = NULL;
	SpectatorVec list;
	SpectatorVec::iterator it;

	g_game.getSpectators(list, oldPos, false, true);
	g_game.getSpectators(list, newPos, true, true);

	std::vector<uint32_t> oldStackPosVector;
	for(it = list.begin(); it != list.end(); ++it){
		if((tmpPlayer = (*it)->getPlayer())){
			oldStackPosVector.push_back(getClientIndexOfThing(tmpPlayer, creature));
		}
	}

	//remove the creature
	__removeThing(actor, creature, 0);

	// Switch the node ownership
	if(qt_node != newTile->qt_node) {
		qt_node->removeCreature(creature);
		newTile->qt_node->addCreature(creature);
	}
	
	//add the creature
	newTile->__addThing(actor, creature);
	int32_t newStackPos = newTile->__getIndexOfThing(creature);

	if(!teleport){
		if(oldPos.y > newPos.y)
			creature->setDirection(NORTH);
		else if(oldPos.y < newPos.y)
			creature->setDirection(SOUTH);
		if(oldPos.x < newPos.x)
			creature->setDirection(EAST);
		else if(oldPos.x > newPos.x)
			creature->setDirection(WEST);
	}

	//send to client
	uint32_t i = 0;
	for(it = list.begin(); it != list.end(); ++it){
		if((tmpPlayer = (*it)->getPlayer())){
			tmpPlayer->sendCreatureMove(creature, newTile, newPos, this, oldPos, oldStackPosVector[i], teleport);
			++i;
		}
	}

	//event method
	for(it = list.begin(); it != list.end(); ++it){
		(*it)->onCreatureMove(creature, newTile, newPos, this, oldPos, teleport);
	}

	postRemoveNotification(actor, creature, toCylinder, oldStackPos, true);
	newTile->postAddNotification(actor, creature, this, newStackPos);

	g_game.onCreatureMove(actor, creature, this, newTile);
}

ReturnValue Tile::__queryAdd(int32_t index, const Thing* thing, uint32_t count,
	uint32_t flags) const
{
	const CreatureVector* creatures = getCreatures();
	const TileItemVector* items = getItemList();

	if(const Creature* creature = thing->getCreature()){
		if(hasBitSet(FLAG_NOLIMIT, flags)){
			return RET_NOERROR;
		}

		if(hasBitSet(FLAG_PATHFINDING, flags)){
			if(floorChange() || positionChange()){
				return RET_NOTPOSSIBLE;
			}
		}

		if(ground == NULL)
			return RET_NOTPOSSIBLE;

		if(const Actor* monster = creature->getActor()){
			if(hasFlag(TILEPROP_PROTECTIONZONE))
				return RET_NOTPOSSIBLE;

			if(floorChange() || positionChange()){
				return RET_NOTPOSSIBLE;
			}

			if(monster->canPushCreatures() && !monster->isSummon()){
				if(creatures){
					Creature* creature;
					for(uint32_t i = 0; i < creatures->size(); ++i){
						creature = creatures->at(i);
						if(creature->getPlayer() && creature->getPlayer()->hasFlag(PlayerFlag_CannotBeSeen))
							continue;

						if( !creature->getActor() ||
							!creature->isPushable() ||
							(creature->getActor()->isPlayerSummon()))
						{
							return RET_NOTPOSSIBLE;
						}
					}
				}
			}
			else if(creatures && !creatures->empty()){
				for(CreatureVector::const_iterator cit = creatures->begin(); cit != creatures->end(); ++cit){
					if(!(*cit)->getPlayer() || !(*cit)->getPlayer()->hasFlag(PlayerFlag_CannotBeSeen)){
						return RET_NOTENOUGHROOM;
					}
				}
			}

			if(hasFlag(TILEPROP_BLOCKSOLIDNOTMOVEABLE)){
				return RET_NOTPOSSIBLE;
			}

			if(hasBitSet(FLAG_PATHFINDING, flags) && (hasFlag(TILEPROP_BLOCKPATHNOTFIELD) && hasFlag(TILEPROP_BLOCKPATHNOTMOVEABLE)) ){
				return RET_NOTPOSSIBLE;
			}

			if(hasFlag(TILEPROP_BLOCKSOLID) || (hasBitSet(FLAG_PATHFINDING, flags) && hasFlag(TILEPROP_BLOCKPATHNOTFIELD))){
				if(!(monster->canPushItems() || hasBitSet(FLAG_IGNOREBLOCKITEM, flags) ) ){
					return RET_NOTPOSSIBLE;
				}
			}

			MagicField* field = getFieldItem();
			if(field && !field->blockSolid()){
				CombatType combatType = field->getCombatType();
				//There is 3 options for a monster to enter a magic field
				//1) Actor is immune
				if(!monster->isImmune(combatType)){
					//1) Actor is "strong" enough to handle the damage
					//2) Actor is already afflicated by this type of condition
					if(hasBitSet(FLAG_IGNOREFIELDDAMAGE, flags)){
						if( !(monster->canPushItems() ||
							monster->hasCondition(Combat::DamageToConditionType(combatType), false)) ){
							return RET_NOTPOSSIBLE;
						}
					}
					else{
						return RET_NOTPOSSIBLE;
					}
				}
			}

			return RET_NOERROR;
		}
		else if(const Player* player = creature->getPlayer()){
			if(creatures && !creatures->empty() && !hasBitSet(FLAG_IGNOREBLOCKCREATURE, flags)){
				for(CreatureVector::const_iterator cit = creatures->begin(); cit != creatures->end(); ++cit){
					if(!(*cit)->getPlayer() || !(*cit)->getPlayer()->hasFlag(PlayerFlag_CannotBeSeen)){
						return RET_NOTENOUGHROOM; //RET_NOTPOSSIBLE
					}
				}
			}

			if(player->getParent() == NULL && hasFlag(TILEPROP_NOLOGOUT)){
				//player is trying to login to a "no logout" tile
				return RET_NOTPOSSIBLE;
			}

			if(player->isPzLocked() && !player->getTile()->hasFlag(TILEPROP_PVPZONE) && hasFlag(TILEPROP_PVPZONE)){
				//player is trying to enter a pvp zone while being pz-locked
				return RET_PLAYERISPZLOCKEDENTERPVPZONE;
			}

			if(player->isPzLocked() && player->getTile()->hasFlag(TILEPROP_PVPZONE) && !hasFlag(TILEPROP_PVPZONE)){
				//player is trying to leave a pvp zone while being pz-locked
				return RET_PLAYERISPZLOCKEDLEAVEPVPZONE;
			}

			if(hasFlag(TILEPROP_NOPVPZONE) && player->isPzLocked()){
				return RET_PLAYERISPZLOCKED;
			}

			if(hasFlag(TILEPROP_PROTECTIONZONE) && player->isPzLocked()){
				return RET_PLAYERISPZLOCKED;
			}
		}
		else{
			if(creatures && !creatures->empty() && !hasBitSet(FLAG_IGNOREBLOCKCREATURE, flags)){
				for(CreatureVector::const_iterator cit = creatures->begin(); cit != creatures->end(); ++cit){
					if(!(*cit)->getPlayer() || !(*cit)->getPlayer()->hasFlag(PlayerFlag_CannotBeSeen)){
						return RET_NOTENOUGHROOM;
					}
				}
			}
		}

		if(items){
			if(!hasBitSet(FLAG_IGNOREBLOCKITEM, flags)){
				//If the FLAG_IGNOREBLOCKITEM bit isn't set we dont have to iterate every single item
				if(hasFlag(TILEPROP_BLOCKSOLID)){
					return RET_NOTENOUGHROOM;
				}
			}
			else{
				//FLAG_IGNOREBLOCKITEM is set
				if(ground){
					const ItemType& iiType = Item::items[ground->getID()];
					if(iiType.blockSolid && (!iiType.moveable || ground->getUniqueId() != 0)){
						return RET_NOTPOSSIBLE;
					}
				}

				if(const TileItemVector* items = getItemList()){
					Item* iitem;
					for(ItemVector::const_iterator it = items->begin(); it != items->end(); ++it){
						iitem = (*it);
						const ItemType& iiType = Item::items[iitem->getID()];
						if(iiType.blockSolid && (!iiType.moveable || iitem->getUniqueId() != 0)){
							return RET_NOTPOSSIBLE;
						}
					}
				}
			}
		}
	}
	else if(const Item* item = thing->getItem()){
#ifdef __DEBUG__
		if(thing->getParent() == NULL && !hasBitSet(FLAG_NOLIMIT, flags)){
			std::cout << "Notice: Tile::__queryAdd() - thing->getParent() == NULL" << std::endl;
		}
#endif
		if(items && items->size() >= 0xFFFF){
			return RET_NOTPOSSIBLE;
		}

		if(hasBitSet(FLAG_NOLIMIT, flags)){
			return RET_NOERROR;
		}

		bool itemIsHangable = item->isHangable();

		if(ground == NULL && !itemIsHangable){
			return RET_NOTPOSSIBLE;
		}

		if(creatures && !creatures->empty() && item->blockSolid() && !hasBitSet(FLAG_IGNOREBLOCKCREATURE, flags)){
			for(CreatureVector::const_iterator cit = creatures->begin(); cit != creatures->end(); ++cit){
				if(!(*cit)->getPlayer() || !(*cit)->getPlayer()->hasFlag(PlayerFlag_CannotBeSeen)){
					return RET_NOTENOUGHROOM;
				}
			}
		}

		bool hasHangable = false;
		bool supportHangable = false;

		if(items){
			Thing* iithing = NULL;
			for(uint32_t i = 0; i < getThingCount(); ++i){
				iithing = __getThing(i);
				if(const Item* iitem = iithing->getItem()){
					const ItemType& iiType = Item::items[iitem->getID()];

					if(iiType.isHangable){
						hasHangable = true;
					}

					if(iiType.isHorizontal || iiType.isVertical){
						supportHangable = true;
					}

					if(itemIsHangable && (iiType.isHorizontal || iiType.isVertical)){
						//
					}
					else if(iiType.blockSolid){
						if(item->isPickupable()){
							if(iiType.allowPickupable){
								continue;
							}

							if(!iiType.hasHeight || iiType.pickupable || iiType.isBed()){
								return RET_NOTENOUGHROOM;
							}
						}
						else{
							return RET_NOTENOUGHROOM;
						}
					}
				}
			}
		}

		if(itemIsHangable && hasHangable && supportHangable){
			return RET_NEEDEXCHANGE;
		}

	}

	return RET_NOERROR;
}

ReturnValue Tile::__queryMaxCount(int32_t index, const Thing* thing, uint32_t count, uint32_t& maxQueryCount,
	uint32_t flags) const
{
	maxQueryCount = std::max((uint32_t)1, count);
	return RET_NOERROR;
}

ReturnValue Tile::__queryRemove(const Thing* thing, uint32_t count, uint32_t flags) const
{
	int32_t index = __getIndexOfThing(thing);

	if(index == -1){
		return RET_NOTPOSSIBLE;
	}

	const Item* item = thing->getItem();
	if(item == NULL){
		return RET_NOTPOSSIBLE;
	}

	if(count == 0 || (item->isStackable() && count > item->getItemCount())){
		return RET_NOTPOSSIBLE;
	}

	if(!item->isMoveable() && !hasBitSet(FLAG_IGNORENOTMOVEABLE, flags)){
		return RET_NOTMOVEABLE;
	}

	return RET_NOERROR;
}

Cylinder* Tile::__queryDestination(int32_t& index, const Thing* thing, Item** destItem,
	uint32_t& flags)
{
	Tile* destTile = NULL;
	*destItem = NULL;

	if(floorChangeDown()){
		int dx = getPosition().x;
		int dy = getPosition().y;
		int dz = getPosition().z + 1;
		Tile* downTile = g_game.getTile(dx, dy, dz);

		if(downTile){
			if(downTile->floorChange(NORTH))
				dy += 1;
			if(downTile->floorChange(SOUTH))
				dy -= 1;
			if(downTile->floorChange(EAST))
				dx -= 1;
			if(downTile->floorChange(WEST))
				dx += 1;
			destTile = g_game.getTile(dx, dy, dz);
		}
	}
	else if(floorChange()){
		int dx = getPosition().x;
		int dy = getPosition().y;
		int dz = getPosition().z - 1;

		if(floorChange(NORTH))
			dy -= 1;
		if(floorChange(SOUTH))
			dy += 1;
		if(floorChange(EAST))
			dx += 1;
		if(floorChange(WEST))
			dx -= 1;
		destTile = g_game.getTile(dx, dy, dz);
	}


	if(destTile == NULL){
		destTile = this;
	}
	else{
		flags |= FLAG_NOLIMIT; //Will ignore that there is blocking items/creatures
	}

	if(destTile){
		Thing* destThing = destTile->getTopDownItem();
		if(destThing)
			*destItem = destThing->getItem();
	}

	return destTile;
}

void Tile::__addThing(Creature* actor, Thing* thing)
{
	__addThing(actor, 0, thing);
}

void Tile::__addThing(Creature* actor, int32_t index, Thing* thing)
{
	Creature* creature = thing->getCreature();
	if(creature){
		g_game.clearSpectatorCache();
		creature->setParent(this);
		CreatureVector* creatures = makeCreatures();
		creatures->insert(creatures->begin(), creature);
		++thingCount;
	}
	else{
		Item* item = thing->getItem();
		if(item == NULL){
#ifdef __DEBUG__MOVESYS__
			std::cout << "Failure: [Tile::__addThing] item == NULL" << std::endl;
			DEBUG_REPORT
#endif
			return /*RET_NOTPOSSIBLE*/;
		}

		TileItemVector* items = getItemList();

		if(items && items->size() > 0xFFFF){
			return /*RET_NOTPOSSIBLE*/;
		}

		item->setParent(this);

		if(item->isGroundTile()){
			if(ground == NULL){
				ground = item;
				++thingCount;
				onAddTileItem(item);
			}
			else{
				const ItemType& oldType = Item::items[ground->getID()];
				const ItemType& newType = Item::items[item->getID()];

				int32_t oldGroundIndex = __getIndexOfThing(ground);
				Item* oldGround = ground;
				ground->setParent(NULL);
				g_game.FreeThing(ground);
				ground = item;
				updateTileFlags(oldGround, true);
				updateTileFlags(item, false);

				onUpdateTileItem(oldGround, oldType, item, newType);
				postRemoveNotification(actor, oldGround, NULL, oldGroundIndex, true);
			}
		}
		else if(item->isAlwaysOnTop()){
			if(item->isSplash()){
				//remove old splash if exists
				if(items){
					for(ItemVector::iterator it = items->getBeginTopItem(); it != items->getEndTopItem(); ++it){
						if((*it)->isSplash()){
							int32_t oldSplashIndex = __getIndexOfThing(*it);
							Item* oldSplash = *it;
							__removeThing(actor, oldSplash, 1);
							oldSplash->setParent(NULL);
							g_game.FreeThing(oldSplash);
							postRemoveNotification(actor, oldSplash, NULL, oldSplashIndex, true);
							break;
						}
					}
				}
			}

			bool isInserted = false;

			if(items){
				for(ItemVector::iterator it = items->getBeginTopItem(); it != items->getEndTopItem(); ++it){
					//Note: this is different from internalAddThing
					if(Item::items[item->getID()].alwaysOnTopOrder <= Item::items[(*it)->getID()].alwaysOnTopOrder){
						items->insert(it, item);
						++thingCount;
						isInserted = true;
						break;
					}
				}
			}
			else{
				items = makeItemList();
			}

			if(!isInserted){
				items->push_back(item);
				++thingCount;
			}

			onAddTileItem(item);
		}
		else{
			if(item->isMagicField()){
				//remove old field item if exists
				if(items){
					MagicField* oldField = NULL;
					for(ItemVector::iterator it = items->getBeginDownItem(); it != items->getEndDownItem(); ++it){
						if((oldField = (*it)->getMagicField())){
							if(oldField->isReplaceable()){
								int32_t oldFieldIndex = __getIndexOfThing(*it);
								__removeThing(actor, oldField, 1);

								oldField->setParent(NULL);
								g_game.FreeThing(oldField);
								postRemoveNotification(actor, oldField, NULL, oldFieldIndex, true);
								break;
							}
							else{
								//This magic field cannot be replaced.
								item->setParent(NULL);
								g_game.FreeThing(item);
								return;
							}
						}
					}
				}

			}

			items = makeItemList();
			items->insert(items->getBeginDownItem(), item);
			++items->downItemCount;
			++thingCount;
			onAddTileItem(item);
		}
	}
}

void Tile::__updateThing(Creature* actor, Thing* thing, uint16_t itemId, uint32_t count)
{
	int32_t index = __getIndexOfThing(thing);
	if(index == -1){
#ifdef __DEBUG__MOVESYS__
		std::cout << "Failure: [Tile::__updateThing] index == -1" << std::endl;
		DEBUG_REPORT
#endif
		return /*RET_NOTPOSSIBLE*/;
	}

	Item* item = thing->getItem();
	if(item == NULL){
#ifdef __DEBUG__MOVESYS__
		std::cout << "Failure: [Tile::__updateThing] item == NULL" << std::endl;
		DEBUG_REPORT
#endif
		return /*RET_NOTPOSSIBLE*/;
	}

	const ItemType& oldType = Item::items[item->getID()];
	const ItemType& newType = Item::items[itemId];

	updateTileFlags(item, true);

	item->setID(itemId);
	item->setSubType(count);

	updateTileFlags(item, false);

	onUpdateTileItem(item, oldType, item, newType);
}

void Tile::__replaceThing(Creature* actor, uint32_t index, Thing* thing)
{
	int32_t pos = index;

	Item* item = thing->getItem();
	if(item == NULL){
#ifdef __DEBUG__MOVESYS__
		std::cout << "Failure: [Tile::__updateThing] item == NULL" << std::endl;
		DEBUG_REPORT
#endif
		return /*RET_NOTPOSSIBLE*/;
	}

	Item* oldItem = NULL;
	bool isInserted = false;

	if(!isInserted && ground){
		if(pos == 0){
			oldItem = ground;
			ground = item;
			isInserted = true;
		}

		--pos;
	}

	TileItemVector* items = getItemList();
	if(items && !isInserted){
		int32_t topItemSize = getTopItemCount();
		if(pos < topItemSize){
			ItemVector::iterator it = items->getBeginTopItem();
			it += pos;

			oldItem = (*it);
			it = items->erase(it);
			items->insert(it, item);
			isInserted = true;
		}

		pos -= topItemSize;
	}

	CreatureVector* creatures = getCreatures();
	if(creatures){
		if(!isInserted && pos < (int32_t)creatures->size()){
	#ifdef __DEBUG__MOVESYS__
			std::cout << "Failure: [Tile::__updateThing] Update object is a creature" << std::endl;
			DEBUG_REPORT
	#endif
			return /*RET_NOTPOSSIBLE*/;
		}

		pos -= (uint32_t)creatures->size();
	}

	if(items && !isInserted){
		int32_t downItemSize = getDownItemCount();
		if(pos < downItemSize){
			ItemVector::iterator it = items->begin();
			it += pos;
			pos = 0;

			oldItem = (*it);
			it = items->erase(it);
			items->insert(it, item);
			isInserted = true;
		}
	}

	if(isInserted){
		item->setParent(this);

		updateTileFlags(oldItem, true);
		updateTileFlags(item, false);
		const ItemType& oldType = Item::items[oldItem->getID()];
		const ItemType& newType = Item::items[item->getID()];
		onUpdateTileItem(oldItem, oldType, item, newType);

		oldItem->setParent(NULL);
		return /*RET_NOERROR*/;
	}

#ifdef __DEBUG__MOVESYS__
	std::cout << "Failure: [Tile::__updateThing] Update object not found" << std::endl;
	DEBUG_REPORT
#endif
}

void Tile::__removeThing(Creature* actor, Thing* thing, uint32_t count)
{
	Creature* creature = thing->getCreature();
	if(creature){
		CreatureVector* creatures = getCreatures();
		if(creatures){
			CreatureVector::iterator it = std::find(creatures->begin(), creatures->end(), thing);

			if(it == creatures->end()){
#ifdef __DEBUG__MOVESYS__
				std::cout << "Failure: [Tile::__removeThing] creature not found" << std::endl;
				DEBUG_REPORT
#endif
				return; //RET_NOTPOSSIBLE;
			}

			g_game.clearSpectatorCache();
			creatures->erase(it);
			--thingCount;
			return;
		}
		else{
#ifdef __DEBUG__MOVESYS__
			std::cout << "Failure: [Tile::__removeThing] creature not found" << std::endl;
			DEBUG_REPORT
#endif
			return; //RET_NOTPOSSIBLE;
		}
	}
	else{
		Item* item = thing->getItem();
		if(item == NULL){
#ifdef __DEBUG__MOVESYS__
			std::cout << "Failure: [Tile::__removeThing] item == NULL" << std::endl;
			DEBUG_REPORT
#endif
			return /*RET_NOTPOSSIBLE*/;
		}

		int32_t index = __getIndexOfThing(item);
		if(index == -1){
#ifdef __DEBUG__MOVESYS__
			std::cout << "Failure: [Tile::__removeThing] index == -1" << std::endl;
			DEBUG_REPORT
#endif
			return /*RET_NOTPOSSIBLE*/;
		}

		if(item == ground){
			const SpectatorVec& list = g_game.getSpectators(getPosition());
			std::vector<uint32_t> oldStackPosVector;

			Player* tmpPlayer = NULL;
			for(SpectatorVec::const_iterator it = list.begin(); it != list.end(); ++it){
				if((tmpPlayer = (*it)->getPlayer())){
					oldStackPosVector.push_back(getClientIndexOfThing(tmpPlayer, ground));
				}
			}
			ground->setParent(NULL);
			ground = NULL;
			--thingCount;
			onRemoveTileItem(list, oldStackPosVector, item);

			return /*RET_NOERROR*/;
		}

		if(item->isAlwaysOnTop()){
			TileItemVector* items = getItemList();
			if(items){
				for(ItemVector::iterator it = items->getBeginTopItem(); it != items->getEndTopItem(); ++it){
					if(*it == item){
						const SpectatorVec& list = g_game.getSpectators(getPosition());
						std::vector<uint32_t> oldStackPosVector;

						Player* tmpPlayer = NULL;
						for(SpectatorVec::const_iterator iit = list.begin(); iit != list.end(); ++iit){
							if((tmpPlayer = (*iit)->getPlayer())){
								oldStackPosVector.push_back(getClientIndexOfThing(tmpPlayer, *it));
							}
						}
						(*it)->setParent(NULL);
						items->erase(it);
						--thingCount;
						onRemoveTileItem(list, oldStackPosVector, item);
						return /*RET_NOERROR*/;
					}
				}
			}
		}
		else{
			TileItemVector* items = getItemList();
			if(items){
				for(ItemVector::iterator it = items->getBeginDownItem(); it != items->getEndDownItem(); ++it){
					if((*it) == item){
						if(item->isStackable() && count != item->getItemCount()){
							uint8_t newCount = (uint8_t)std::max((int32_t)0, (int32_t)(item->getItemCount() - count));
							updateTileFlags(item, true);
							item->setItemCount(newCount);
							updateTileFlags(item, false);

							const ItemType& it = Item::items[item->getID()];
							onUpdateTileItem(item, it, item, it);
						}
						else{
							const SpectatorVec& list = g_game.getSpectators(getPosition());
							std::vector<uint32_t> oldStackPosVector;

							Player* tmpPlayer = NULL;
							for(SpectatorVec::const_iterator iit = list.begin(); iit != list.end(); ++iit){
								if((tmpPlayer = (*iit)->getPlayer())){
									oldStackPosVector.push_back(getClientIndexOfThing(tmpPlayer, *it));
								}
							}

							(*it)->setParent(NULL);
							items->erase(it);
							--items->downItemCount;
							--thingCount;
							onRemoveTileItem(list, oldStackPosVector, item);
						}

						return /*RET_NOERROR*/;
					}
				}
			}
		}
	}
#ifdef __DEBUG__MOVESYS__
	std::cout << "Failure: [Tile::__removeThing] thing not found" << std::endl;
	DEBUG_REPORT
#endif
}

int32_t Tile::getClientIndexOfThing(const Player* player, const Thing* thing) const
{
	int n = -1;

	if(ground){
		if(ground == thing){
			return 0;
		}

		++n;
	}

	const TileItemVector* items = getItemList();
	if(items){
		if(thing->getItem()){
			for(ItemVector::const_iterator it = items->getBeginTopItem(); it != items->getEndTopItem(); ++it){
				++n;
				if((*it) == thing)
					return n;
			}
		}
		else{
			n += items->getTopItemCount();
		}
	}

	if(const CreatureVector* creatures = getCreatures()){
		for(CreatureVector::const_iterator cit = creatures->begin(); cit != creatures->end(); ++cit){
			if((*cit) == thing || player->canSeeCreature(*cit)){
				++n;
			}
			if((*cit) == thing)
				return n;
		}
	}

	if(items){
		if(thing->getItem()){
			for(ItemVector::const_iterator it = items->getBeginDownItem(); it != items->getEndDownItem(); ++it){
				++n;
				if((*it) == thing)
					return n;
			}
		}
		else{
			n += items->getDownItemCount();
		}
	}

	return -1;
}

int32_t Tile::__getIndexOfThing(const Thing* thing) const
{
	int n = -1;

	if(ground){
		if(ground == thing){
			return 0;
		}

		++n;
	}

	const TileItemVector* items = getItemList();
	if(items){
		if(thing->getItem()){
			for(ItemVector::const_iterator it = items->getBeginTopItem(); it != items->getEndTopItem(); ++it){
				++n;
				if((*it) == thing)
					return n;
			}
		}
		else{
			n += items->getTopItemCount();
		}
	}

	if(const CreatureVector* creatures = getCreatures()){
		for(CreatureVector::const_iterator cit = creatures->begin(); cit != creatures->end(); ++cit){
			++n;
			if((*cit) == thing)
				return n;
		}
	}

	if(items){
		if(thing->getItem()){
			for(ItemVector::const_iterator it = items->getBeginDownItem(); it != items->getEndDownItem(); ++it){
				++n;
				if((*it) == thing)
					return n;
			}
		}
		else{
			n += items->getDownItemCount();
		}
	}

	return -1;
}

int32_t Tile::__getFirstIndex() const
{
	return 0;
}

int32_t Tile::__getLastIndex() const
{
	return getThingCount();
}

uint32_t Tile::__getItemTypeCount(uint16_t itemId, int32_t subType /*= -1*/, bool itemCount /*= true*/) const
{
	uint32_t count = 0;
	Thing* thing = NULL;
	for(uint32_t i = 0; i < getThingCount(); ++i){
		thing = __getThing(i);

		if(const Item* item = thing->getItem()){
			if(item->getID() == itemId && (subType == -1 || subType == item->getSubType())){

				if(itemCount){
					count+= item->getItemCount();
				}
				else{
					if(item->isRune()){
						count+= item->getCharges();
					}
					else{
						count+= item->getItemCount();
					}
				}
			}
		}
	}

	return count;
}

Thing* Tile::__getThing(uint32_t index) const
{
	if(ground){
		if(index == 0){
			return ground;
		}

		--index;
	}

	const TileItemVector* items = getItemList();
	if(items){
		uint32_t topItemSize = items->getTopItemCount();
		if(index < topItemSize)
			return items->at(items->downItemCount + index);

		index -= topItemSize;
	}

	if(const CreatureVector* creatures = getCreatures()){
		if(index < (uint32_t)creatures->size())
			return creatures->at(index);

		index -= (uint32_t)creatures->size();
	}

	if(items){
		if(index < items->getDownItemCount())
			return items->at(index);
	}

	return NULL;
}

void Tile::postAddNotification(Creature* actor, Thing* thing, const Cylinder* oldParent, int32_t index, cylinderlink_t link /*= LINK_OWNER*/)
{
	const Position& cylinderMapPos = getPosition();

	const SpectatorVec& list = g_game.getSpectators(cylinderMapPos);
	SpectatorVec::const_iterator it;

	Player* tmpPlayer = NULL;
	for(it = list.begin(); it != list.end(); ++it){
		if((tmpPlayer = (*it)->getPlayer())){
			tmpPlayer->postAddNotification(actor, thing, oldParent, index, LINK_NEAR);
		}
	}

	//add a reference to this item, it may be deleted after being added (mailbox for example)
	thing->addRef();

	if(link == LINK_OWNER){
		//calling movement scripts
		Creature* creature = thing->getCreature();
		if(creature){
			const Tile* fromTile = NULL;
			if(oldParent){
				fromTile = oldParent->getTile();
			}
		}
		else{
			Item* item = thing->getItem();
			if(item){
				g_game.onItemMove(actor, item, this, true);
			}
		}

		if(hasFlag(TILEPROP_TELEPORT)){
			Teleport* teleport = getTeleportItem();
			if(teleport){
				teleport->__addThing(actor, thing);
			}
		}
		else if(hasFlag(TILEPROP_TRASHHOLDER)){
			TrashHolder* trashholder = getTrashHolder();
			if(trashholder){
				trashholder->__addThing(actor, thing);
			}
		}
		else if(hasFlag(TILEPROP_MAILBOX)){
			Mailbox* mailbox = getMailbox();
			if(mailbox){
				mailbox->__addThing(actor, thing);
			}
		}
	}

	//release the reference to this item onces we are finished
	g_game.FreeThing(thing);
}

void Tile::postRemoveNotification(Creature* actor, Thing* thing,  const Cylinder* newParent, int32_t index, bool isCompleteRemoval, cylinderlink_t link /*= LINK_OWNER*/)
{
	const Position& cylinderMapPos = getPosition();

	const SpectatorVec& list = g_game.getSpectators(cylinderMapPos);
	SpectatorVec::const_iterator it;

	if(/*isCompleteRemoval &&*/ getThingCount() > 8){
		onUpdateTile();
	}

	Player* tmpPlayer = NULL;
	for(it = list.begin(); it != list.end(); ++it){
		if((tmpPlayer = (*it)->getPlayer())){
			tmpPlayer->postRemoveNotification(actor, thing, newParent, index, isCompleteRemoval, LINK_NEAR);
		}
	}

	//calling movement scripts
	Creature* creature = thing->getCreature();
	if(creature){
		const Tile* toTile = NULL;
		if(newParent){
			toTile = newParent->getTile();
		}
	}
	else{
		Item* item = thing->getItem();
		if(item){
			g_game.onItemMove(actor, item, this, false);
		}
	}
}

void Tile::__internalAddThing(Thing* thing)
{
	__internalAddThing(0, thing);
}

void Tile::__internalAddThing(uint32_t index, Thing* thing)
{
	thing->setParent(this);

	Creature* creature = thing->getCreature();
	if(creature){
		g_game.clearSpectatorCache();
		CreatureVector* creatures = makeCreatures();
		creatures->insert(creatures->begin(), creature);
		++thingCount;
	}
	else{
		Item* item = thing->getItem();

		if(item == NULL)
			return;

		TileItemVector* items = makeItemList();

		if(items && items->size() >= 0xFFFF){
			return /*RET_NOTPOSSIBLE*/;
		}

		if(item->isGroundTile()){
			if(ground == NULL){
				ground = item;
				++thingCount;
			}
		}
		else if(item->isAlwaysOnTop()){
			bool isInserted = false;
			for(ItemVector::iterator it = items->getBeginTopItem(); it != items->getEndTopItem(); ++it){
				if(Item::items[(*it)->getID()].alwaysOnTopOrder > Item::items[item->getID()].alwaysOnTopOrder){
					items->insert(it, item);
					++thingCount;
					isInserted = true;
					break;
				}
			}

			if(!isInserted){
				items->push_back(item);
				++thingCount;
			}
		}
		else{
			items->insert(items->getBeginDownItem(), item);
			++items->downItemCount;
			++thingCount;
		}

		updateTileFlags(item, false);
	}
}

bool Tile::hasItemWithProperty(uint32_t props) const
{
	if(ground && ground->hasProperty(props)){
		return true;
	}

	if(const TileItemVector* items = getItemList()){
		for(ItemVector::const_iterator it = items->begin(); it != items->end(); ++it){
			if((*it)->hasProperty(props))
				return true;
		}
	}

	return false;
}

bool Tile::hasItemWithProperty(Item* exclude, uint32_t props) const
{
	assert(exclude);
	if(ground && exclude != ground && ground->hasProperty(props)){
		return true;
	}

	if(const TileItemVector* items = getItemList()){
		for(ItemVector::const_iterator it = items->begin(); it != items->end(); ++it){
			Item* item = *it;
			if(item != exclude && item->hasProperty(props))
				return true;
		}
	}

	return false;
}

void Tile::updateTileFlags(Item* item, bool removed)
{
	if(!removed){
		if(!hasFlag(TILEPROP_FLOORCHANGE)){
			if(item->hasProperty(ITEMPROP_FLOORCHANGEDOWN)){
				setFlag(TILEPROP_FLOORCHANGE);
				setFlag(TILEPROP_FLOORCHANGE_DOWN);
			}
			if(item->hasProperty(ITEMPROP_FLOORCHANGENORTH)){
				setFlag(TILEPROP_FLOORCHANGE);
				setFlag(TILEPROP_FLOORCHANGE_NORTH);
			}
			if(item->hasProperty(ITEMPROP_FLOORCHANGESOUTH)){
				setFlag(TILEPROP_FLOORCHANGE);
				setFlag(TILEPROP_FLOORCHANGE_SOUTH);
			}
			if(item->hasProperty(ITEMPROP_FLOORCHANGEEAST)){
				setFlag(TILEPROP_FLOORCHANGE);
				setFlag(TILEPROP_FLOORCHANGE_EAST);
			}
			if(item->hasProperty(ITEMPROP_FLOORCHANGEWEST)){
				setFlag(TILEPROP_FLOORCHANGE);
				setFlag(TILEPROP_FLOORCHANGE_WEST);
			}
		}

		if(item->hasProperty(ITEMPROP_BLOCKSOLID)){
			setFlag(TILEPROP_BLOCKSOLID);

			if(!item->hasProperty(ITEMPROP_MOVEABLE)){
				setFlag(TILEPROP_BLOCKSOLIDNOTMOVEABLE);
			}
		}
		if(item->hasProperty(ITEMPROP_BLOCKPROJECTILE)){
			setFlag(TILEPROP_BLOCKPATH);

			if(!item->hasProperty(ITEMPROP_MOVEABLE)){
				setFlag(TILEPROP_BLOCKPATHNOTMOVEABLE);
			}

			if(!item->getMagicField()){
				setFlag(TILEPROP_BLOCKPATHNOTFIELD);
			}
		}
		if(item->hasProperty(ITEMPROP_BLOCKPROJECTILE)){
			setFlag(TILEPROP_BLOCKPROJECTILE);
		}
		if(item->hasProperty(ITEMPROP_ISVERTICAL)){
			setFlag(TILEPROP_VERTICAL);
		}
		if(item->hasProperty(ITEMPROP_ISHORIZONTAL)){
			setFlag(TILEPROP_HORIZONTAL);
		}
		if(item->getTeleport()){
			setFlag(TILEPROP_TELEPORT);
		}
		if(item->getMagicField()){
			setFlag(TILEPROP_MAGICFIELD);
		}
		if(item->getMailbox()){
			setFlag(TILEPROP_MAILBOX);
		}
		if(item->getTrashHolder()){
			setFlag(TILEPROP_TRASHHOLDER);
		}
		if(item->getBed()){
			setFlag(TILEPROP_BED);
		}
	}
	else{
		if(item->hasProperty(ITEMPROP_FLOORCHANGEDOWN) && !hasItemWithProperty(item, ITEMPROP_FLOORCHANGEDOWN) ){
			resetFlag(TILEPROP_FLOORCHANGE);
			resetFlag(TILEPROP_FLOORCHANGE_DOWN);
		}
		if(item->hasProperty(ITEMPROP_FLOORCHANGENORTH) && !hasItemWithProperty(item, ITEMPROP_FLOORCHANGENORTH) ){
			resetFlag(TILEPROP_FLOORCHANGE);
			resetFlag(TILEPROP_FLOORCHANGE_NORTH);
		}
		if(item->hasProperty(ITEMPROP_FLOORCHANGESOUTH) && !hasItemWithProperty(item, ITEMPROP_FLOORCHANGESOUTH) ){
			resetFlag(TILEPROP_FLOORCHANGE);
			resetFlag(TILEPROP_FLOORCHANGE_SOUTH);
		}
		if(item->hasProperty(ITEMPROP_FLOORCHANGEEAST) && !hasItemWithProperty(item, ITEMPROP_FLOORCHANGEEAST) ){
			resetFlag(TILEPROP_FLOORCHANGE);
			resetFlag(TILEPROP_FLOORCHANGE_EAST);
		}
		if(item->hasProperty(ITEMPROP_FLOORCHANGEWEST) && !hasItemWithProperty(item, ITEMPROP_FLOORCHANGEWEST) ){
			resetFlag(TILEPROP_FLOORCHANGE);
			resetFlag(TILEPROP_FLOORCHANGE_WEST);
		}

		if(item->hasProperty(ITEMPROP_BLOCKSOLID) && !hasItemWithProperty(item, ITEMPROP_BLOCKSOLID) ){
			resetFlag(TILEPROP_BLOCKSOLID);

			if(!item->hasProperty(ITEMPROP_MOVEABLE) && !hasItemWithProperty(item, ITEMPROP_MOVEABLE) ){
				resetFlag(TILEPROP_BLOCKSOLIDNOTMOVEABLE);
			}
		}
		if(item->hasProperty(ITEMPROP_BLOCKPROJECTILE) && !hasItemWithProperty(item, ITEMPROP_BLOCKPROJECTILE) ){
			resetFlag(TILEPROP_BLOCKPATH);

			if(!item->hasProperty(ITEMPROP_MOVEABLE) && !hasItemWithProperty(item, ITEMPROP_MOVEABLE) ){
				resetFlag(TILEPROP_BLOCKPATHNOTMOVEABLE);
			}

			if(!item->getMagicField()){
				resetFlag(TILEPROP_BLOCKPATHNOTFIELD);
			}
		}
		if(item->hasProperty(ITEMPROP_BLOCKPROJECTILE) && !hasItemWithProperty(item, ITEMPROP_BLOCKPROJECTILE)){
			resetFlag(TILEPROP_BLOCKPROJECTILE);
		}
		if(item->hasProperty(ITEMPROP_ISVERTICAL) && !hasItemWithProperty(item, ITEMPROP_ISVERTICAL) ){
			resetFlag(TILEPROP_VERTICAL);
		}
		if(item->hasProperty(ITEMPROP_ISHORIZONTAL) && !hasItemWithProperty(item, ITEMPROP_ISHORIZONTAL) ){
			resetFlag(TILEPROP_HORIZONTAL);
		}
		if(item->getTeleport()){
			resetFlag(TILEPROP_TELEPORT);
		}
		if(item->getMagicField()){
			resetFlag(TILEPROP_MAGICFIELD);
		}
		if(item->getMailbox()){
			resetFlag(TILEPROP_MAILBOX);
		}
		if(item->getTrashHolder()){
			resetFlag(TILEPROP_TRASHHOLDER);
		}
		if(item->getBed()){
			resetFlag(TILEPROP_BED);
		}
	}
}
