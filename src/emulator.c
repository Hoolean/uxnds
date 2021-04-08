#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#include "uxn.h"
#include "ppu.h"
#include "apu.h"

int initapu(Uxn *u, Uint8 id);

static SDL_AudioDeviceID audio_id;
static SDL_Window *gWindow;
static SDL_Renderer *gRenderer;
static SDL_Texture *gTexture;
static Ppu ppu;
static Apu apu;
static Device *devsystem, *devscreen, *devmouse, *devkey, *devctrl, *devapu;

#pragma mark - Helpers

/* clang-format off */
int  clamp(int val, int min, int max) { return (val >= min) ? (val <= max) ? val : max : min; }
void setflag(Uint8 *a, char flag, int b) { if(b) *a |= flag; else *a &= (~flag); }
/* clang-format on */

#pragma mark - Core

int
error(char *msg, const char *err)
{
	printf("Error %s: %s\n", msg, err);
	return 0;
}

static void
audio_callback(void *u, Uint8 *stream, int len)
{
	apu_render(&apu, (Uxn *)u, (Sint16 *)stream, len >> 2);
}

void
redraw(Uint32 *dst, Uxn *u)
{
	drawppu(&ppu);
	if(ppu.debugger)
		drawdebugger(&ppu, u->wst.dat, u->wst.ptr);
	SDL_UpdateTexture(gTexture, NULL, dst, ppu.width * sizeof(Uint32));
	SDL_RenderClear(gRenderer);
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
	SDL_RenderPresent(gRenderer);
	ppu.reqdraw = 0;
}

void
toggledebug(Uxn *u)
{
	ppu.debugger = !ppu.debugger;
	redraw(ppu.output, u);
}

void
togglezoom(Uxn *u)
{
	ppu.zoom = ppu.zoom == 3 ? 1 : ppu.zoom + 1;
	SDL_SetWindowSize(gWindow, ppu.width * ppu.zoom, ppu.height * ppu.zoom);
	redraw(ppu.output, u);
}

void
quit(void)
{
	free(ppu.output);
	free(ppu.fg);
	free(ppu.bg);
	SDL_UnlockAudioDevice(audio_id);
	SDL_DestroyTexture(gTexture);
	gTexture = NULL;
	SDL_DestroyRenderer(gRenderer);
	gRenderer = NULL;
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	SDL_Quit();
	exit(0);
}

int
init(Uxn *u)
{
	SDL_AudioSpec as;
	if(!initppu(&ppu, 48, 32, 16))
		return error("PPU", "Init failure");
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
		return error("Init", SDL_GetError());
	gWindow = SDL_CreateWindow("Uxn", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ppu.width * ppu.zoom, ppu.height * ppu.zoom, SDL_WINDOW_SHOWN);
	if(gWindow == NULL)
		return error("Window", SDL_GetError());
	gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
	if(gRenderer == NULL)
		return error("Renderer", SDL_GetError());
	gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, ppu.width, ppu.height);
	if(gTexture == NULL)
		return error("Texture", SDL_GetError());
	SDL_StartTextInput();
	SDL_ShowCursor(SDL_DISABLE);
	SDL_zero(as);
	as.freq = SAMPLE_FREQUENCY;
	as.format = AUDIO_S16;
	as.channels = 2;
	as.callback = audio_callback;
	as.samples = 2048;
	as.userdata = u;
	audio_id = SDL_OpenAudioDevice(NULL, 0, &as, NULL, 0);
	if(!audio_id)
		return error("Audio", SDL_GetError());
	SDL_PauseAudioDevice(audio_id, 0);
	return 1;
}

void
domouse(Uxn *u, SDL_Event *event)
{
	Uint8 flag = 0x00;
	Uint16 addr = devmouse->addr + 2;
	Uint16 x = clamp(event->motion.x / ppu.zoom - ppu.pad, 0, ppu.hor * 8 - 1);
	Uint16 y = clamp(event->motion.y / ppu.zoom - ppu.pad, 0, ppu.ver * 8 - 1);
	mempoke16(u, addr + 0, x);
	mempoke16(u, addr + 2, y);
	u->ram.dat[addr + 5] = 0x00;
	flag = event->button.button == SDL_BUTTON_LEFT ? 0x01 : 0x10;
	switch(event->type) {
	case SDL_MOUSEBUTTONDOWN:
		u->ram.dat[addr + 4] |= flag;
		if(flag == 0x10 && (u->ram.dat[addr + 4] & 0x01))
			u->ram.dat[addr + 5] = 0x01;
		if(flag == 0x01 && (u->ram.dat[addr + 4] & 0x10))
			u->ram.dat[addr + 5] = 0x10;
		break;
	case SDL_MOUSEBUTTONUP:
		u->ram.dat[addr + 4] &= (~flag);
		break;
	}
}

void
dotext(Uxn *u, SDL_Event *event)
{
	int i;
	Uint16 addr = devkey->addr + 2;
	if(SDL_GetModState() & KMOD_LCTRL || SDL_GetModState() & KMOD_RCTRL)
		return;
	for(i = 0; i < SDL_TEXTINPUTEVENT_TEXT_SIZE; ++i) {
		char c = event->text.text[i];
		if(c < ' ' || c > '~')
			break;
		u->ram.dat[addr] = c;
	}
}

void
doctrl(Uxn *u, SDL_Event *event, int z)
{
	Uint8 flag = 0x00;
	Uint16 addr = devctrl->addr + 2;
	if(z && event->key.keysym.sym == SDLK_h) {
		if(SDL_GetModState() & KMOD_LCTRL)
			toggledebug(u);
		if(SDL_GetModState() & KMOD_LALT)
			togglezoom(u);
	}
	switch(event->key.keysym.sym) {
	case SDLK_LCTRL: flag = 0x01; break;
	case SDLK_LALT: flag = 0x02; break;
	case SDLK_BACKSPACE: flag = 0x04; break;
	case SDLK_RETURN: flag = 0x08; break;
	case SDLK_UP: flag = 0x10; break;
	case SDLK_DOWN: flag = 0x20; break;
	case SDLK_LEFT: flag = 0x40; break;
	case SDLK_RIGHT: flag = 0x80; break;
	}
	setflag(&u->ram.dat[addr], flag, z);
}

#pragma mark - Devices

Uint8
console_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat;
	switch(b0) {
	case 0x08: printf("%c", b1); break;
	case 0x09: printf("0x%02x\n", b1); break;
	case 0x0b: printf("0x%04x\n", (m[ptr + 0x0a] << 8) + b1); break;
	case 0x0d: printf("%s\n", &m[(m[ptr + 0x0c] << 8) + b1]); break;
	}
	fflush(stdout);
	return b1;
}

Uint8
screen_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat;
	ptr += 8;
	if(b0 == 0x0c) {
		Uint16 x = (m[ptr] << 8) + m[ptr + 1];
		Uint16 y = (m[ptr + 2] << 8) + m[ptr + 3];
		putpixel(&ppu, b1 >> 4 & 0xf ? ppu.fg : ppu.bg, x, y, b1 & 0xf);
		ppu.reqdraw = 1;
	}
	return b1;
}

Uint8
sprite_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat;
	ptr += 8;
	if(b0 == 0x0e) {
		Uint16 v, h;
		Uint16 x = (m[ptr] << 8) + m[ptr + 1];
		Uint16 y = (m[ptr + 2] << 8) + m[ptr + 3];
		Uint8 blend = b1 & 0xf;
		Uint8 *layer = ((b1 >> 4) & 0xf) % 2 ? ppu.fg : ppu.bg;
		Uint8 *sprite = &m[(m[ptr + 4] << 8) + m[ptr + 5]];
		for(v = 0; v < 8; v++)
			for(h = 0; h < 8; h++) {
				Uint8 ch1 = ((sprite[v] >> (7 - h)) & 0x1);
				if(ch1 == 0 && (blend == 0x05 || blend == 0x0a || blend == 0x0f))
					continue;
				putpixel(&ppu, layer, x + h, y + v, ch1 ? blend % 4 : blend / 4);
			}
		ppu.reqdraw = 1;
	}
	return b1;
}

Uint8
file_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat;
	char *name = (char *)&m[(m[ptr + 8] << 8) + m[ptr + 8 + 1]];
	Uint16 length = (m[ptr + 8 + 2] << 8) + m[ptr + 8 + 3];
	Uint16 offset = (m[ptr + 0] << 8) + m[ptr + 1];
	if(b0 == 0x0d) {
		Uint16 addr = (m[ptr + 8 + 4] << 8) + b1;
		FILE *f = fopen(name, "r");
		if(f && fseek(f, offset, SEEK_SET) != -1 && fread(&m[addr], length, 1, f)) {
			fclose(f);
			printf("Loaded %d bytes, at %04x from %s\n", length, addr, name);
		}
	} else if(b0 == 0x0f) {
		Uint16 addr = (m[ptr + 8 + 6] << 8) + b1;
		FILE *f = fopen(name, (m[ptr + 2] & 0x1) ? "a" : "w");
		if(f && fseek(f, offset, SEEK_SET) != -1 && fwrite(&m[addr], length, 1, f)) {
			fclose(f);
			printf("Saved %d bytes, at %04x from %s\n", length, addr, name);
		}
	}
	return b1;
}

static Uint8
audio_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat + ptr;
	if(b0 == 0xa) {
		if(b1 >= apu.n_notes) apu.notes = SDL_realloc(apu.notes, (b1 + 1) * sizeof(Note));
		while(b1 >= apu.n_notes) SDL_zero(apu.notes[apu.n_notes++]);
		apu_play_note(&apu.notes[b1], (m[0x0] << 8) + m[0x1], (m[0x2] << 8) + m[0x3], m[0x8], m[0x9]);
	} else if(b0 == 0xe && apu.queue != NULL) {
		if(apu.queue->n == apu.queue->sz) {
			apu.queue->sz = apu.queue->sz < 4 ? 4 : apu.queue->sz * 2;
			apu.queue->dat = SDL_realloc(apu.queue->dat, apu.queue->sz * sizeof(*apu.queue->dat));
		}
		apu.queue->dat[apu.queue->n++] = (m[0xb] << 8) + m[0xc];
		apu.queue->dat[apu.queue->n++] = (m[0xd] << 8) + b1;
	} else if(b0 == 0xf && apu.queue != NULL)
		apu.queue->finishes = 1;
	return b1;
}

Uint8
midi_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	(void)u;
	printf("%04x - %02x,%02x\n", ptr, b0, b1);
	return b1;
}

Uint8
datetime_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat;
	time_t seconds = time(NULL);
	struct tm *t = localtime(&seconds);
	t->tm_year += 1900;
	m[ptr + 0] = (t->tm_year & 0xff00) >> 8;
	m[ptr + 1] = t->tm_year & 0xff;
	m[ptr + 2] = t->tm_mon;
	m[ptr + 3] = t->tm_mday;
	m[ptr + 4] = t->tm_hour;
	m[ptr + 5] = t->tm_min;
	m[ptr + 6] = t->tm_sec;
	m[ptr + 7] = t->tm_wday;
	m[ptr + 8] = (t->tm_yday & 0xff00) >> 8;
	m[ptr + 9] = t->tm_yday & 0xff;
	m[ptr + 10] = t->tm_isdst;
	(void)b0;
	return b1;
}

Uint8
system_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat;
	m[PAGE_DEVICE + b0] = b1;
	loadtheme(&ppu, &m[PAGE_DEVICE + 0x0008]);
	(void)ptr;
	return b1;
}

Uint8
ppnil(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	(void)u;
	(void)ptr;
	(void)b0;
	return b1;
}

#pragma mark - Generics

int
start(Uxn *u)
{
	inituxn(u, 0x0200);
	redraw(ppu.output, u);
	while(1) {
		SDL_Event event;
		double elapsed, start = SDL_GetPerformanceCounter();
		SDL_LockAudioDevice(audio_id);
		while(SDL_PollEvent(&event) != 0) {
			switch(event.type) {
			case SDL_QUIT:
				quit();
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				doctrl(u, &event, event.type == SDL_KEYDOWN);
				evaluxn(u, devctrl->vector);
				break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEMOTION:
				domouse(u, &event);
				evaluxn(u, devmouse->vector);
				break;
			case SDL_TEXTINPUT:
				dotext(u, &event);
				evaluxn(u, devkey->vector);
				break;
			case SDL_WINDOWEVENT:
				if(event.window.event == SDL_WINDOWEVENT_EXPOSED)
					redraw(ppu.output, u);
				break;
			}
		}
		evaluxn(u, devscreen->vector);
		SDL_UnlockAudioDevice(audio_id);
		if(ppu.reqdraw)
			redraw(ppu.output, u);
		elapsed = (SDL_GetPerformanceCounter() - start) / (double)SDL_GetPerformanceFrequency() * 1000.0f;
		SDL_Delay(clamp(16.666f - elapsed, 0, 1000));
	}
	return 1;
}

int
main(int argc, char **argv)
{
	Uxn u;
	ppu.zoom = 2;

	if(argc < 2)
		return error("Input", "Missing");
	if(!bootuxn(&u))
		return error("Boot", "Failed");
	if(!loaduxn(&u, argv[1]))
		return error("Load", "Failed");
	if(!init(&u))
		return error("Init", "Failed");

	devsystem = portuxn(&u, 0x00, "system", system_poke);
	portuxn(&u, 0x01, "console", console_poke);
	devscreen = portuxn(&u, 0x02, "screen", screen_poke);
	portuxn(&u, 0x03, "sprite", sprite_poke);
	devctrl = portuxn(&u, 0x04, "controller", ppnil);
	devkey = portuxn(&u, 0x05, "key", ppnil);
	devmouse = portuxn(&u, 0x06, "mouse", ppnil);
	portuxn(&u, 0x07, "file", file_poke);
	devapu = portuxn(&u, 0x08, "audio", audio_poke);
	apu.channel_addr = devapu->addr + 0xa;
	portuxn(&u, 0x09, "midi", ppnil);
	portuxn(&u, 0x0a, "datetime", datetime_poke);
	portuxn(&u, 0x0b, "---", ppnil);
	portuxn(&u, 0x0c, "---", ppnil);
	portuxn(&u, 0x0d, "---", ppnil);
	portuxn(&u, 0x0e, "---", ppnil);
	portuxn(&u, 0x0f, "---", ppnil);

	/* Write screen size to dev/screen */
	mempoke16(&u, devscreen->addr + 2, ppu.hor * 8);
	mempoke16(&u, devscreen->addr + 4, ppu.ver * 8);

	start(&u);
	quit();
	return 0;
}
