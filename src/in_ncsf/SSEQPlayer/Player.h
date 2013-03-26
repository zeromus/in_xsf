/*
 * SSEQ Player - Player structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2013-03-21
 *
 * Adapted from source code of FeOS Sound System
 * By fincs
 * https://github.com/fincs/FSS
 */

#ifndef SSEQPLAYER_PLAYER_H
#define SSEQPLAYER_PLAYER_H

#include "SSEQ.h"
#include "Track.h"
#include "Channel.h"
#include "consts.h"

struct Player
{
	uint8_t prio, nTracks;
	uint16_t tempo, tempoCount, tempoRate /* 8.8 fixed point */;
	int16_t masterVol;

	const SSEQ *sseq;

	uint8_t trackIds[FSS_TRACKCOUNT];
	Track tracks[FSS_MAXTRACKS];
	Channel channels[16];

	uint32_t sampleRate;
	Interpolation interpolation;

	Player();

	bool Setup(const SSEQ *sseq);
	void ClearState();
	int ChannelAlloc(int type, int prio);
	int TrackAlloc();
	void Run();
	void UpdateTracks();
	void Timer();
};

#endif
