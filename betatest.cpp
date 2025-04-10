#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>
#include <algorithm>
#include "fssimplewindow.h"
#include "ysglfontdata.h"
#include "yssimplesound.h"
#include "yspng.h"

#define MYPI 3.1415926535
#define nCan 10
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 800
#define PIRATE_SHIP_SPEED 0.1
#define PIRATE_SHIP_INTERVAL 150
const double worldWidth = 100.0;
const double worldHeight = 80.0;
double xScale = WINDOW_WIDTH / worldWidth;
double yScale = WINDOW_HEIGHT / worldHeight;
class SoundManager {
private:
	YsSoundPlayer player;
	YsSoundPlayer::SoundData introMusic;
	YsSoundPlayer::SoundData gameMusic;
	YsSoundPlayer::SoundData outroMusic;
	YsSoundPlayer::SoundData cannonSound;
	YsSoundPlayer::SoundData collisionSound;
	YsSoundPlayer::SoundData enemyCannonFireSound;
public:
	bool Initialize();
	void PlayIntroMusic();
	void StopIntroMusic();
	void PlayGameMusic();
	void StopGameMusic();
	void PlayOutroMusic();
	void StopOutroMusic();
	void PlayCannonFireSound();
	void PlayCollisionSound();
	void KeepPlaying();
};
bool SoundManager::Initialize() {
	FsChangeToProgramDir();
	player.Start();
	bool allLoaded = true;
	if (YSOK != introMusic.LoadWav("IntroMusic.wav")) {
		printf("Debug: Failed to load IntroMusic.wav\n");
		allLoaded = false;
	}
	if (YSOK != gameMusic.LoadWav("InGameMusic.wav")) {
		printf("Debug: Failed to load InGameMusic.wav\n");
		allLoaded = false;
	}
	if (YSOK != outroMusic.LoadWav("OutroMusic.wav")) {
		printf("Debug: Failed to load OutroMusic.wav\n");
		allLoaded = false;
	}
	if (YSOK != cannonSound.LoadWav("CannonFire.wav")) {
		printf("Debug: Failed to load CannonFire.wav\n");
		allLoaded = false;
	}
	if (YSOK != collisionSound.LoadWav("Collision.wav")) {
		printf("Debug: Failed to load Collision.wav\n");
		allLoaded = false;
	}
	return allLoaded;
}
void SoundManager::PlayIntroMusic() {
	player.Stop(introMusic);
	player.PlayBackground(introMusic);
}
void SoundManager::StopIntroMusic() {
	player.Stop(introMusic);
}
void SoundManager::PlayGameMusic() {
	player.Stop(gameMusic);
	player.PlayBackground(gameMusic);
}
void SoundManager::StopGameMusic() {
	player.Stop(gameMusic);
}
void SoundManager::PlayOutroMusic() {
	player.Stop(outroMusic);
	player.PlayBackground(outroMusic);
}
void SoundManager::StopOutroMusic() {
	player.Stop(outroMusic);
}
void SoundManager::PlayCannonFireSound() {
	player.PlayOneShot(cannonSound);
}
void SoundManager::PlayCollisionSound() {
	player.PlayOneShot(collisionSound);
}
void SoundManager::KeepPlaying() {
	player.KeepPlaying();
}
class Obstacle {
public:
	double x, y;
	double radius;
	std::vector<std::pair<double, double>> offsets;
	Obstacle(double x_, double y_, double radius_);
	void DrawObstacle() const;
};
Obstacle::Obstacle(double x_, double y_, double radius_) : x(x_), y(y_),
radius(radius_) {
	for (int i = 0; i <= 360; i += 12) {
		double a = (double)i * MYPI / 180.0;
		double variation = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
		double current_radius = radius + variation;
		double ox = cos(a) * current_radius * xScale;
		double oy = sin(a) * current_radius * yScale;
		offsets.emplace_back(ox, oy);
	}
}
void Obstacle::DrawObstacle() const {
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(0.95f, 0.78f, 0.39f);
	for (const auto& offset : offsets) {
		glVertex2d(x * xScale + offset.first, (worldHeight - y) * yScale +
			offset.second);
	}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(0.0f, 0.5f, 0.0f);
	int isleCount = 0;
	for (const auto& offset : offsets) {
		if (isleCount <= 8) {
			glVertex2d(x * xScale + (offset.first - 2), (worldHeight - y) * yScale
				+ (offset.second - 2));
		}
		else if (isleCount > 8 && isleCount <= 16) {
			glVertex2d(x * xScale + (offset.first + 2), (worldHeight - y) * yScale
				+ (offset.second - 2));
		}
		else if (isleCount > 16 && isleCount <= 23) {
			glVertex2d(x * xScale + (offset.first + 2), (worldHeight - y) * yScale
				+ (offset.second + 2));
		}
		else if (isleCount > 23) {
			glVertex2d(x * xScale + (offset.first - 2), (worldHeight - y) * yScale
				+ (offset.second + 2));
		}
		++isleCount;
	}
	glEnd();
}
class PirateShip {
public:
	void DrawShip(double x, double y, double theta, bool enorfr);
};
void PirateShip::DrawShip(double x, double y, double theta, bool enorfr) {
	double th = theta * (MYPI / 180.0);
	double pX = x * xScale;
	double pY = (worldHeight - y) * yScale;
	double scale = 1.75 * ((xScale + yScale) / 2.0) / 10.0;
	const int hullPoints = 16;
	double Hullx[hullPoints] = { -1,-1,-3,-5,-7,-8,-8,-4,4,8,8,7,5,3,1,1 };
	double Hully[hullPoints] = { -24,-20,-19,-17,-12,-8,8,20,20,8,-8,-12,-17,-19,-
	20,-24 };
	glColor3f(0.545f, 0.271f, 0.075f);
	glBegin(GL_TRIANGLE_FAN);
	for (int j = 0; j < hullPoints; ++j) {
		double hx = Hullx[j];
		double hy = Hully[j];
		double h = sqrt((hx * hx) + (hy * hy));
		double phi = atan2(hy, hx);
		glVertex2d(pX + scale * (h * cos(th + phi)), pY - scale * (h * sin(th +
			phi)));
	}
	glEnd();
	glColor3f(0.824f, 0.706f, 0.549f);
	glBegin(GL_TRIANGLE_FAN);
	double deckx[14] = { -1,-3,-4,-6,-7,-7,-3,3,7,7,6,4,3,1 };
	double decky[14] = { -19,-18,-17,-12,-8,8,19,19,8,-8,-12,-17,-18,-19 };
	for (int j = 0; j < 14; ++j) {
		double dx = deckx[j];
		double dy = decky[j];
		double d = sqrt(dx * dx + dy * dy);
		double phi = atan2(dy, dx);
		glVertex2d(pX + scale * (d * cos(th + phi)), pY - scale * (d * sin(th +
			phi)));
	}
	glEnd();
	if (enorfr == true) {
		glColor3f(1.0, 1.0, 1.0);
	}
	else {
		glColor3f(0.531, 0.0, 0.082);
	}
	glBegin(GL_TRIANGLE_FAN);
	double bsailx[6] = { -16,-15,-14,14,15,16 };
	double bsaily1[6] = { -7,-9,-10,-10,-9,-7 };
	for (int j = 0; j < 6; ++j) {
		double mx = bsailx[j];
		double my = bsaily1[j];
		double m = sqrt(mx * mx + my * my);
		double phi = atan2(my, mx);
		glVertex2d(pX + scale * (m * cos(th + phi)), pY - scale * (m * sin(th +
			phi)));
	}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	double bsaily2[6] = { 6,4,3,3,4,6 };
	for (int j = 0; j < 6; ++j) {
		double mx = bsailx[j];
		double my = bsaily2[j];
		double m = sqrt(mx * mx + my * my);
		double phi = atan2(my, mx);
		glVertex2d(pX + scale * (m * cos(th + phi)), pY - scale * (m * sin(th +
			phi)));
	}
	glEnd();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINE_LOOP);
	for (int j = 0; j < 6; ++j) {
		double mx = bsailx[j];
		double my = bsaily1[j];
		double m = sqrt(mx * mx + my * my);
		double phi = atan2(my, mx);
		glVertex2d(pX + scale * (m * cos(th + phi)), pY - scale * (m * sin(th +
			phi)));
	}
	glEnd();
	glBegin(GL_LINE_LOOP);
	for (int j = 0; j < 6; ++j) {
		double mx = bsailx[j];
		double my = bsaily2[j];
		double m = sqrt(mx * mx + my * my);
		double phi = atan2(my, mx);
		glVertex2d(pX + scale * (m * cos(th + phi)), pY - scale * (m * sin(th +
			phi)));
	}
	glEnd();
	if (enorfr == true) {
		glColor3f(1.0, 1.0, 1.0);
	}
	else {
		glColor3f(0.531, 0.0, 0.082);
	}
	glBegin(GL_TRIANGLE_FAN);
	double msailx[6] = { -12,-11,-10,10,11,12 };
	double msaily1[6] = { -5,-7,-8,-8,-7,-5 };
	for (int j = 0; j < 6; ++j) {
		double mx = msailx[j];
		double my = msaily1[j];
		double m = sqrt(mx * mx + my * my);
		double phi = atan2(my, mx);
		glVertex2d(pX + scale * (m * cos(th + phi)), pY - scale * (m * sin(th +
			phi)));
	}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	double msaily2[6] = { 8,6,5,5,6,8 };
	for (int j = 0; j < 6; ++j) {
		double mx = msailx[j];
		double my = msaily2[j];
		double m = sqrt(mx * mx + my * my);
		double phi = atan2(my, mx);
		glVertex2d(pX + scale * (m * cos(th + phi)), pY - scale * (m * sin(th +
			phi)));
	}
	glEnd();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINE_LOOP);
	for (int j = 0; j < 6; ++j) {
		double mx = msailx[j];
		double my = msaily1[j];
		double m = sqrt(mx * mx + my * my);
		double phi = atan2(my, mx);
		glVertex2d(pX + scale * (m * cos(th + phi)), pY - scale * (m * sin(th +
			phi)));
	}
	glEnd();
	glBegin(GL_LINE_LOOP);
	for (int j = 0; j < 6; ++j) {
		double mx = msailx[j];
		double my = msaily2[j];
		double m = sqrt(mx * mx + my * my);
		double phi = atan2(my, mx);
		glVertex2d(pX + scale * (m * cos(th + phi)), pY - scale * (m * sin(th +
			phi)));
	}
	glEnd();
	if (enorfr == true) {
		glColor3f(1.0, 1.0, 1.0);
	}
	else {
		glColor3f(0.531, 0.0, 0.082);
	}
	glBegin(GL_TRIANGLE_FAN);
	double tsailx[6] = { -8,-7,-6,6,7,8 };
	double tsaily1[6] = { -3,-5,-6,-6,-5,-3 };
	for (int j = 0; j < 6; ++j) {
		double mx = tsailx[j];
		double my = tsaily1[j];
		double m = sqrt(mx * mx + my * my);
		double phi = atan2(my, mx);
		glVertex2d(pX + scale * (m * cos(th + phi)), pY - scale * (m * sin(th +
			phi)));
	}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	double tsaily2[6] = { 10,8,7,7,8,10 };
	for (int j = 0; j < 6; ++j) {
		double mx = tsailx[j];
		double my = tsaily2[j];
		double m = sqrt(mx * mx + my * my);
		double phi = atan2(my, mx);
		glVertex2d(pX + scale * (m * cos(th + phi)), pY - scale * (m * sin(th +
			phi)));
	}
	glEnd();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINE_LOOP);
	for (int j = 0; j < 6; ++j) {
		double mx = tsailx[j];
		double my = tsaily1[j];
		double m = sqrt(mx * mx + my * my);
		double phi = atan2(my, mx);
		glVertex2d(pX + scale * (m * cos(th + phi)), pY - scale * (m * sin(th +
			phi)));
	}
	glEnd();
	glBegin(GL_LINE_LOOP);
	for (int j = 0; j < 6; ++j) {
		double mx = tsailx[j];
		double my = tsaily2[j];
		double m = sqrt(mx * mx + my * my);
		double phi = atan2(my, mx);
		glVertex2d(pX + scale * (m * cos(th + phi)), pY - scale * (m * sin(th +
			phi)));
	}
	glEnd();
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_TRIANGLE_FAN);
	double cannonLx[7] = { 0,1,4,10,10,4,1 };
	double cannony[7] = { 0,-1,-2,-2,2,2,1 };
	for (int j = 0; j < 7; ++j) {
		double cx = cannonLx[j];
		double cy = cannony[j];
		double c = sqrt(cx * cx + cy * cy);
		double phi = atan2(cy, cx);
		glVertex2d(pX + scale * (c * cos(th + phi)), pY - scale * (c * sin(th +
			phi)));
	}
	double cannonRx[7] = { 0,-1,-4,-10,-10,-4,-1 };
	for (int j = 0; j < 7; ++j) {
		double cx = cannonRx[j];
		double cy = cannony[j];
		double c = sqrt(cx * cx + cy * cy);
		double phi = atan2(cy, cx);
		glVertex2d(pX + scale * (c * cos(th + phi)), pY - scale * (c * sin(th +
			phi)));
	}
	glEnd();
}
class Cannonball {
public:
	double cx[nCan], cy[nCan], cPhi[nCan];
	bool cState[nCan];
	int dir[nCan];
	Cannonball() {
		CannonIntialize();
	}
	void CannonIntialize();
	void DrawCannonBall(double cx, double cy, int r);
	void Fire(double sx, double sy, double theta, int direction);
	void CannonMove();
	void CannonRender();
};
void Cannonball::CannonIntialize() {
	for (int i = 0; i < nCan; ++i) {
		cx[i] = cy[i] = cPhi[i] = 1.0;
		dir[i] = 0;
		cState[i] = false;
	}
}
void Cannonball::DrawCannonBall(double cx, double cy, int r) {
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex2d(cx * xScale, (worldHeight - cy) * yScale);
	for (int i = 0; i <= 360; i += 5) {
		double a = (double)i * MYPI / 180.0;
		double x = cos(a) * r;
		double y = sin(a) * r;
		glVertex2d(cx * xScale + x, (worldHeight - cy) * yScale + y);
	}
	glEnd();
}
void Cannonball::Fire(double sx, double sy, double theta, int direction) {
	for (int i = 0; i < nCan; ++i) {
		if (!cState[i]) {
			cState[i] = true;
			dir[i] = direction;
			if (direction == 0) {
				cx[i] = sx + 1.5 * sin((theta - 90.0) * (MYPI / 180.0));
				cy[i] = sy - 1.5 * cos((theta - 90.0) * (MYPI / 180.0));
			}
			else {
				cx[i] = sx + 1.5 * sin((theta + 90.0) * (MYPI / 180.0));
				cy[i] = sy - 1.5 * cos((theta + 90.0) * (MYPI / 180.0));
			}
			cPhi[i] = theta;
			break;
		}
	}
}
void Cannonball::CannonMove() {
	for (int i = 0; i < nCan; ++i) {
		if (cState[i]) {
			if (dir[i] == 0) {
				cx[i] += sin((cPhi[i] - 90.0) * (MYPI / 180.0));
				cy[i] -= cos((cPhi[i] - 90.0) * (MYPI / 180.0));
			}
			else {
				cx[i] += sin((cPhi[i] + 90.0) * (MYPI / 180.0));
				cy[i] -= cos((cPhi[i] + 90.0) * (MYPI / 180.0));
			}
		}
	}
}
void Cannonball::CannonRender() {
	for (int i = 0; i < nCan; ++i) {
		if (cx[i] > worldWidth || cx[i] < 0.0 || cy[i] > worldHeight || cy[i] <
			0.0) {
			cState[i] = false;
		}
		if (cState[i]) {
			DrawCannonBall(cx[i], cy[i], 5);
		}
	}
}
class EnemyCannonball {
public:
	double cx[nCan], cy[nCan], cPhi[nCan];
	bool cState[nCan];
	EnemyCannonball() {
		Initialize();
	}
	void Initialize();
	void DrawCannonBall(double cx, double cy, int r);
	void Fire(double sx, double sy, double theta);
	void Move();
	void Render();
};
void EnemyCannonball::Initialize() {
	for (int i = 0; i < nCan; ++i) {
		cx[i] = cy[i] = cPhi[i] = 1.0;
		cState[i] = false;
	}
}
void EnemyCannonball::DrawCannonBall(double cx, double cy, int r) {
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2d(cx * xScale, (worldHeight - cy) * yScale);
	for (int i = 0; i <= 360; i += 5) {
		double a = (double)i * MYPI / 180.0;
		double x = cos(a) * r;
		double y = sin(a) * r;
		glVertex2d(cx * xScale + x, (worldHeight - cy) * yScale + y);
	}
	glEnd();
}
void EnemyCannonball::Fire(double sx, double sy, double theta) {
	for (int i = 0; i < nCan; ++i) {
		if (!cState[i]) {
			cState[i] = true;
			cx[i] = sx;
			cy[i] = sy;
			cPhi[i] = theta;
			break;
		}
	}
}
void EnemyCannonball::Move() {
	for (int i = 0; i < nCan; ++i) {
		if (cState[i]) {
			double speed = 0.2;
			cx[i] += sin((cPhi[i] + 90.0) * (MYPI / 180.0)) * speed;
			cy[i] -= cos((cPhi[i] + 90.0) * (MYPI / 180.0)) * speed;
		}
	}
}
void EnemyCannonball::Render() {
	for (int i = 0; i < nCan; ++i) {
		if (cx[i] > worldWidth || cx[i] < 0.0 || cy[i] > worldHeight || cy[i] <
			0.0) {
			cState[i] = false;
		}
		if (cState[i]) {
			DrawCannonBall(cx[i], cy[i], 5);
		}
	}
}
class EnemyShip {
public:
	double x;
	double y;
	double angle;
	bool alive;
};
class EnemyLogic {
public:
	std::vector<EnemyShip> enemies;
	int spawnInterval;
	int frameCounter;
	void InitializeEnemies(int numEnemies);
	void UpdateAndRenderEnemies(Cannonball& playerCannonballs, int& score, const
		std::vector<Obstacle>& obstacles);
	bool AllEnemiesDestroyed();
	bool PlayerDefeated(double playerX, double playerY);
};
void EnemyLogic::InitializeEnemies(int numEnemies) {
	enemies.clear();
	for (int i = 0; i < numEnemies; ++i) {
		EnemyShip ship;
		ship.x = worldWidth + i * PIRATE_SHIP_INTERVAL / 10.0;
		ship.y = rand() % int(worldHeight);
		ship.angle = 270.0;
		ship.alive = true;
		enemies.push_back(ship);
	}
	frameCounter = 0;
	spawnInterval = PIRATE_SHIP_INTERVAL;
}
void EnemyLogic::UpdateAndRenderEnemies(Cannonball& playerCannonballs, int& score,
	const std::vector<Obstacle>& obstacles) {
	frameCounter++;
	if (frameCounter % spawnInterval == 0) {
		EnemyShip ship;
		ship.x = worldWidth;
		ship.y = rand() % int(worldHeight);
		ship.angle = 270.0;
		ship.alive = true;
		enemies.push_back(ship);
	}
	for (auto& ship : enemies) {
		if (ship.alive) {
			ship.x -= PIRATE_SHIP_SPEED;
			for (int i = 0; i < nCan; ++i) {
				if (playerCannonballs.cState[i]) {
					double dx = ship.x - playerCannonballs.cx[i];
					double dy = ship.y - playerCannonballs.cy[i];
					double dist = sqrt(dx * dx + dy * dy);
					if (dist < 3.0) {
						ship.alive = false;
						playerCannonballs.cState[i] = false;
						score++;
					}
				}
			}
			for (const auto& obs : obstacles) {
				double dx = ship.x - obs.x;
				double dy = ship.y - obs.y;
				double dist = sqrt(dx * dx + dy * dy);
				if (dist < obs.radius + 2.0) {
					ship.alive = false;
				}
			}
			if (ship.alive) {
				PirateShip enemyShipDrawer;
				enemyShipDrawer.DrawShip(ship.x, ship.y, ship.angle, false);
			}
		}
	}
	enemies.erase(
		std::remove_if(enemies.begin(), enemies.end(),
			[](const EnemyShip& ship) { return ship.x < -10.0 || !ship.alive; }),
		enemies.end());
}
bool EnemyLogic::AllEnemiesDestroyed() {
	return enemies.empty();
}
bool EnemyLogic::PlayerDefeated(double playerX, double playerY) {
	for (const auto& ship : enemies) {
		double dx = ship.x - playerX;
		double dy = ship.y - playerY;
		double dist = sqrt(dx * dx + dy * dy);
		if (dist < 3.0) {
			return true;
		}
	}
	return false;
}
class UIManager {
public:
	std::string playerName;
	int windowWidth;
	int windowHeight;
private:
	YsRawPngDecoder background;
	GLuint BKTextureID;
	YsRawPngDecoder background2;
	GLuint BKTextureID2;
	YsRawPngDecoder victoryBackground;
	GLuint VictoryTextureID;
	YsRawPngDecoder defeatBackground;
	GLuint DefeatTextureID;
	YsRawPngDecoder leaderboardBackground;
	GLuint LeaderboardTextureID;
public:
	UIManager(int width, int height);
	~UIManager();
	bool ShowStartScreen();
	void ShowEndScreen(bool victory, int score, SoundManager& soundManager);
	void AskPlayerName();
	void DisplayLeaderboard();
	void SaveScore(int score);
	std::vector<std::pair<std::string, int>> ReadPlayerScores();
};
UIManager::UIManager(int width, int height) {
	windowWidth = width;
	windowHeight = height;
	if (YSOK != background.Decode("Background.png")) {
		printf("Failed to load Background.png\n");
	}
	else {
		background.Flip();
		glGenTextures(1, &BKTextureID);
		glBindTexture(GL_TEXTURE_2D, BKTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, background.wid, background.hei, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, background.rgba);
	}
	if (YSOK != background2.Decode("background2.png")) {
		printf("Failed to load Background2.png\n");
	}
	else {
		background2.Flip();
		glGenTextures(1, &BKTextureID2);
		glBindTexture(GL_TEXTURE_2D, BKTextureID2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, background2.wid, background2.hei,
			0, GL_RGBA, GL_UNSIGNED_BYTE, background2.rgba);
	}
	if (YSOK != victoryBackground.Decode("Victory.png")) {
		printf("Failed to load Victory.png\n");
	}
	else {
		victoryBackground.Flip();
		glGenTextures(1, &VictoryTextureID);
		glBindTexture(GL_TEXTURE_2D, VictoryTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, victoryBackground.wid,
			victoryBackground.hei, 0, GL_RGBA, GL_UNSIGNED_BYTE, victoryBackground.rgba);
	}
	if (YSOK != defeatBackground.Decode("Defeat.png")) {
		printf("Failed to load Defeat.png\n");
	}
	else {
		defeatBackground.Flip();
		glGenTextures(1, &DefeatTextureID);
		glBindTexture(GL_TEXTURE_2D, DefeatTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, defeatBackground.wid,
			defeatBackground.hei, 0, GL_RGBA, GL_UNSIGNED_BYTE, defeatBackground.rgba);
	}
	if (YSOK != leaderboardBackground.Decode("Leaderboard.png")) {
		printf("Failed to load Leaderboard.png\n");
	}
	else {
		leaderboardBackground.Flip();
		glGenTextures(1, &LeaderboardTextureID);
		glBindTexture(GL_TEXTURE_2D, LeaderboardTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, leaderboardBackground.wid,
			leaderboardBackground.hei, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			leaderboardBackground.rgba);
	}
}
UIManager::~UIManager() {
	glDeleteTextures(1, &BKTextureID);
	glDeleteTextures(1, &BKTextureID2);
	glDeleteTextures(1, &VictoryTextureID);
	glDeleteTextures(1, &DefeatTextureID);
	glDeleteTextures(1, &LeaderboardTextureID);
}
bool UIManager::ShowStartScreen() {
	for (;;) {
		FsPollDevice();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, BKTextureID);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i(0, 0);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2i(windowWidth, 0);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i(windowWidth, windowHeight);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(0, windowHeight);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glColor3ub(255, 215, 0);
		int titleFontHeight = 48;
		int titleWidth = 16 * 32;
		glRasterPos2i((windowWidth - titleWidth) / 2, windowHeight * 0.15 + 20);
		YsGlDrawFontBitmap32x48("Captain Gigabyte");
		int buttonWid = int(windowWidth * 0.27);
		int buttonHei = int(windowHeight * 0.1);
		int buttonXOffset = -270;
		int buttonYOffset = 80;
		int startButtonX = windowWidth / 2 - buttonWid / 2 + buttonXOffset;
		int startButtonY = windowHeight / 2 - buttonHei / 2 - int(windowHeight *
			0.15) + buttonYOffset;
		int leaderboardButtonX = windowWidth / 2 - buttonWid / 2 + buttonXOffset;
		int leaderboardButtonY = windowHeight / 2 - buttonHei / 2 + buttonYOffset;
		int exitButtonX = windowWidth / 2 - buttonWid / 2 + buttonXOffset;
		int exitButtonY = windowHeight / 2 - buttonHei / 2 + int(windowHeight *
			0.15) + buttonYOffset;
		glColor3ub(255, 215, 0);
		glLineWidth(2);
		glBegin(GL_LINE_LOOP);
		glVertex2i(startButtonX, startButtonY + buttonHei);
		glVertex2i(startButtonX + buttonWid, startButtonY + buttonHei);
		glVertex2i(startButtonX + buttonWid, startButtonY);
		glVertex2i(startButtonX, startButtonY);
		glEnd();
		glColor3ub(70, 130, 180);
		glBegin(GL_QUADS);
		glVertex2i(startButtonX + 1, startButtonY + 1);
		glVertex2i(startButtonX + buttonWid - 1, startButtonY + 1);
		glVertex2i(startButtonX + buttonWid - 1, startButtonY + buttonHei - 1);
		glVertex2i(startButtonX + 1, startButtonY + buttonHei - 1);
		glEnd();
		glColor3ub(255, 255, 255);
		std::string startText = "Start New Game";
		int textWidth = startText.length() * 12;
		int textHeight = 20;
		glRasterPos2i(startButtonX + (buttonWid - textWidth) / 2 - 20, startButtonY
			+ (buttonHei + textHeight) / 2 - 3);
		YsGlDrawFontBitmap16x20(startText.c_str());
		glColor3ub(255, 215, 0);
		glBegin(GL_LINE_LOOP);
		glVertex2i(leaderboardButtonX, leaderboardButtonY + buttonHei);
		glVertex2i(leaderboardButtonX + buttonWid, leaderboardButtonY + buttonHei);
		glVertex2i(leaderboardButtonX + buttonWid, leaderboardButtonY);
		glVertex2i(leaderboardButtonX, leaderboardButtonY);
		glEnd();
		glColor3ub(70, 130, 180);
		glBegin(GL_QUADS);
		glVertex2i(leaderboardButtonX + 1, leaderboardButtonY + 1);
		glVertex2i(leaderboardButtonX + buttonWid - 1, leaderboardButtonY + 1);
		glVertex2i(leaderboardButtonX + buttonWid - 1, leaderboardButtonY +
			buttonHei - 1);
		glVertex2i(leaderboardButtonX + 1, leaderboardButtonY + buttonHei - 1);
		glEnd();
		glColor3ub(255, 255, 255);
		std::string leaderboardText = "Leaderboard";
		textWidth = leaderboardText.length() * 12;
		glRasterPos2i(leaderboardButtonX + (buttonWid - textWidth) / 2 - 17,
			leaderboardButtonY + (buttonHei + textHeight) / 2 - 3);
		YsGlDrawFontBitmap16x20(leaderboardText.c_str());
		glColor3ub(255, 215, 0);
		glBegin(GL_LINE_LOOP);
		glVertex2i(exitButtonX, exitButtonY + buttonHei);
		glVertex2i(exitButtonX + buttonWid, exitButtonY + buttonHei);
		glVertex2i(exitButtonX + buttonWid, exitButtonY);
		glVertex2i(exitButtonX, exitButtonY);
		glEnd();
		glColor3ub(70, 130, 180);
		glBegin(GL_QUADS);
		glVertex2i(exitButtonX + 1, exitButtonY + 1);
		glVertex2i(exitButtonX + buttonWid - 1, exitButtonY + 1);
		glVertex2i(exitButtonX + buttonWid - 1, exitButtonY + buttonHei - 1);
		glVertex2i(exitButtonX + 1, exitButtonY + buttonHei - 1);
		glEnd();
		glColor3ub(255, 255, 255);
		std::string exitText = "Exit";
		textWidth = exitText.length() * 12;
		glRasterPos2i(exitButtonX + (buttonWid - textWidth) / 2 - 7, exitButtonY +
			(buttonHei + textHeight) / 2 - 3);
		YsGlDrawFontBitmap16x20(exitText.c_str());
		int lb, mb, rb, mx, my;
		FsPollDevice();
		FsGetMouseState(lb, mb, rb, mx, my);
		if (lb) {
			if (mx > startButtonX && mx < startButtonX + buttonWid &&
				my > startButtonY && my < startButtonY + buttonHei) {
				AskPlayerName();
				return true;
			}
			else if (mx > leaderboardButtonX && mx < leaderboardButtonX + buttonWid
				&&
				my > leaderboardButtonY && my < leaderboardButtonY + buttonHei) {
				DisplayLeaderboard();
			}
			else if (mx > exitButtonX && mx < exitButtonX + buttonWid &&
				my > exitButtonY && my < exitButtonY + buttonHei) {
				return false;
			}
		}
		FsSwapBuffers();
		FsSleep(25);
		if (FsInkey() == FSKEY_ESC) {
			return false;
		}
	}
}
void UIManager::AskPlayerName() {
	std::string str;
	auto t0 = std::chrono::high_resolution_clock::now();
	while (FsInkey() != FSKEY_NULL) {}
	while (FsInkeyChar() != 0) {}
	for (;;) {
		FsPollDevice();
		auto key = FsInkey();
		if (FSKEY_ENTER == key) {
			if (str.empty()) {
			}
			else if (str.size() > 30) {
			}
			else {
				playerName = str;
				break;
			}
		}
		else if (FSKEY_BS == key && str.size() > 0) {
			str.pop_back();
		}
		char c = FsInkeyChar();
		if (' ' <= c && c <= 127) {
			str.push_back(c);
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, BKTextureID2);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i(0, 0);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2i(windowWidth, 0);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i(windowWidth, windowHeight);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(0, windowHeight);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glColor3ub(255, 215, 0);
		int titleFontHeight = 28;
		int titleWidth = 16 * 16;
		glRasterPos2i((windowWidth - titleWidth) / 2, windowHeight * 0.15 + 20);
		YsGlDrawFontBitmap20x28("Captain Gigabyte");
		glColor3ub(255, 255, 255);
		std::string prompt = "Please enter your name:";
		int promptWidth = prompt.length() * 12;
		glRasterPos2i((windowWidth - promptWidth) / 2, windowHeight / 2 - 50);
		YsGlDrawFontBitmap16x20(prompt.c_str());
		int nameWidth = str.length() * 12;
		glRasterPos2i((windowWidth - nameWidth) / 2, windowHeight / 2);
		auto dt = std::chrono::high_resolution_clock::now() - t0;
		auto ms =
			std::chrono::duration_cast<std::chrono::milliseconds>(dt).count();
		int flag = (ms / 500) % 2;
		if (0 != flag) {
			str.push_back('_');
		}
		else {
			str.push_back(' ');
		}
		YsGlDrawFontBitmap16x20(str.c_str());
		str.pop_back();
		FsSwapBuffers();
		FsSleep(25);
	}
}
bool compareScore(const std::pair<std::string, int>& a, const
	std::pair<std::string, int>& b) {
	return a.second > b.second;
}
void UIManager::DisplayLeaderboard() {
	auto scores = ReadPlayerScores();
	std::sort(scores.begin(), scores.end(), compareScore);
	const int maxDisplay = 8;
	for (;;) {
		FsPollDevice();
		auto key = FsInkey();
		if (key == FSKEY_ESC) {
			break;
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, LeaderboardTextureID);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
		glTexCoord2f(1.0f, 1.0f); glVertex2i(windowWidth, 0);
		glTexCoord2f(1.0f, 0.0f); glVertex2i(windowWidth, windowHeight);
		glTexCoord2f(0.0f, 0.0f); glVertex2i(0, windowHeight);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		int x = int(windowWidth * 0.3);
		int y = int(windowHeight * 0.3);
		int lineHeight = 35;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4ub(0, 0, 0, 150);
		glBegin(GL_QUADS);
		glVertex2i(0, 0);
		glVertex2i(0, windowHeight);
		glVertex2i(windowWidth, windowHeight);
		glVertex2i(windowWidth, 0);
		glEnd();
		glDisable(GL_BLEND);
		glColor3ub(255, 215, 0);
		int titleFontHeight = 28;
		int titleWidth = 10 * 16;
		glRasterPos2i((windowWidth - titleWidth) / 2, windowHeight * 0.15);
		YsGlDrawFontBitmap20x28("Leaderboard");
		glColor3ub(255, 255, 255);
		if (!scores.empty()) {
			int rank = 1;
			for (size_t i = 0; i < scores.size() && rank <= maxDisplay; ++i) {
				std::string player = scores[i].first;
				int score = scores[i].second;
				std::string line = std::to_string(rank) + ". " + player + ": " +
					std::to_string(score);
				glRasterPos2i(x, y + rank * lineHeight);
				YsGlDrawFontBitmap16x20(line.c_str());
				rank++;
			}
		}
		else {
			glRasterPos2i(x + 10, y);
			YsGlDrawFontBitmap16x20("Leaderboard is empty!");
			glRasterPos2i(x + 10, y + lineHeight);
			YsGlDrawFontBitmap16x20("Create your legend here!");
		}
		glRasterPos2i(int(windowWidth * 0.75), int(windowHeight * 0.95));
		YsGlDrawFontBitmap10x14("Press ESC to go back");
		FsSwapBuffers();
		FsSleep(10);
	}
}
void UIManager::SaveScore(int score) {
	FILE* fp;
	fp = fopen("PlayerScores.txt", "a");
	if (fp != NULL) {
		std::string modifiedName = playerName;
		std::replace(modifiedName.begin(), modifiedName.end(), ' ', '_');
		fprintf(fp, "%s %d\n", modifiedName.c_str(), score);
		fclose(fp);
	}
}
std::vector<std::pair<std::string, int>> UIManager::ReadPlayerScores() {
	FILE* fp;
	std::vector<std::pair<std::string, int>> scores;
	fp = fopen("PlayerScores.txt", "r");
	if (fp != NULL) {
		char str[256];
		while (fgets(str, sizeof(str), fp) != NULL) {
			std::string line(str);
			std::istringstream iss(line);
			std::string name;
			int score;
			if (iss >> name >> score) {
				std::replace(name.begin(), name.end(), '_', ' ');
				scores.emplace_back(name, score);
			}
		}
		fclose(fp);
	}
	return scores;
}
void UIManager::ShowEndScreen(bool victory, int score, SoundManager& soundManager)
{
	soundManager.PlayOutroMusic();
	for (;;) {
		FsPollDevice();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_TEXTURE_2D);
		if (victory) {
			glBindTexture(GL_TEXTURE_2D, VictoryTextureID);
		}
		else {
			glBindTexture(GL_TEXTURE_2D, DefeatTextureID);
		}
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i(0, 0);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2i(windowWidth, 0);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i(windowWidth, windowHeight);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(0, windowHeight);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4ub(0, 0, 0, 150);
		glBegin(GL_QUADS);
		glVertex2i(0, 0);
		glVertex2i(0, windowHeight);
		glVertex2i(windowWidth, windowHeight);
		glVertex2i(windowWidth, 0);
		glEnd();
		glDisable(GL_BLEND);
		glColor3ub(255, 215, 0);
		int titleFontHeight = 28;
		int titleWidth = 16 * 16;
		glRasterPos2i((windowWidth - titleWidth) / 2, windowHeight * 0.15 + 20);
		YsGlDrawFontBitmap20x28("Captain Gigabyte");
		glColor3ub(victory ? 0 : 255, victory ? 255 : 0, 0);
		std::string statusText = victory ? "Victory!" : "Defeat!";
		int statusWidth = statusText.length() * 16;
		glRasterPos2i((windowWidth - statusWidth) / 2, windowHeight / 2 - 60);
		YsGlDrawFontBitmap20x28(statusText.c_str());
		glColor3ub(255, 255, 255);
		std::string nameText = "Player: " + playerName;
		int nameWidth = nameText.length() * 12;
		glRasterPos2i((windowWidth - nameWidth) / 2, windowHeight / 2 - 20);
		YsGlDrawFontBitmap16x20(nameText.c_str());
		std::string scoreText = "Score: " + std::to_string(score);
		int scoreWidth = scoreText.length() * 12;
		glRasterPos2i((windowWidth - scoreWidth) / 2, windowHeight / 2 + 20);
		YsGlDrawFontBitmap16x20(scoreText.c_str());
		std::string exitPrompt = "Press ESC to Exit";
		int exitWidth = exitPrompt.length() * 12;
		glRasterPos2i((windowWidth - exitWidth) / 2, windowHeight / 2 + 60);
		YsGlDrawFontBitmap16x20(exitPrompt.c_str());
		soundManager.KeepPlaying();
		FsSwapBuffers();
		FsSleep(25);
		if (FsInkey() == FSKEY_ESC) {
			break;
		}
	}
	soundManager.StopOutroMusic();
}
bool IsPlayerCollidingWithObstacles(double x, double y, const
	std::vector<Obstacle>& obstacles) {
	for (const auto& obs : obstacles) {
		double dx = obs.x - x;
		double dy = obs.y - y;
		double dist = sqrt(dx * dx + dy * dy);
		if (dist < (obs.radius + 2.0)) {
			return true;
		}
	}
	return false;
}
void FindSafeStartingPosition(double& x, double& y, const std::vector<Obstacle>&
	obstacles) {
	bool safe = false;
	while (!safe) {
		x = 5.0 + (double)(rand() % int(worldWidth - 10.0));
		y = 5.0 + (double)(rand() % int(worldHeight - 10.0));
		safe = !IsPlayerCollidingWithObstacles(x, y, obstacles);
	}
}
int main(void) {
	FsOpenWindow(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 1);
	srand(unsigned int(time(NULL)));
	SoundManager soundManager;
	if (!soundManager.Initialize()) {
		printf("Failed to initialize sound manager. Some sounds may not play.\n");
	}
	UIManager uiManager(WINDOW_WIDTH, WINDOW_HEIGHT);
	soundManager.PlayIntroMusic();
	bool exitGame = false;
	while (!exitGame) {
		if (!uiManager.ShowStartScreen()) {
			break;
		}
		soundManager.StopIntroMusic();
		soundManager.PlayGameMusic();
		PirateShip playerShip;
		Cannonball cannonball;
		cannonball.CannonIntialize();
		EnemyLogic enemyLogic;
		enemyLogic.InitializeEnemies(10);
		EnemyCannonball enemyCannonball;
		enemyCannonball.Initialize();
		std::vector<Obstacle> obstacles;
		int numObstacles = 6;
		for (int i = 0; i < numObstacles; ++i) {
			double obs_x = 20.0 + (double)(rand() % int(worldWidth - 40.0));
			double obs_y = 10.0 + (double)(rand() % int(worldHeight - 20.0));
			double obs_radius = 1.0;
			Obstacle obs(obs_x, obs_y, obs_radius);
			obstacles.push_back(obs);
		}
		double theta = 90.0;
		double sx, sy;
		FindSafeStartingPosition(sx, sy, obstacles);
		bool terminate = false;
		int score = 0;
		bool ECupDown = true;
		int enemyFireInterval = 80;
		int enemyFireCounter = 0;
		int playerFireInterval = 25;
		int playerFireCounterLeft = playerFireInterval;
		int playerFireCounterRight = playerFireInterval;
		while (!terminate) {
			glClearColor(0.0f, 0.0f, 0.9f, 0.0f);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			FsPollDevice();
			int key = FsInkey();
			if (key == FSKEY_ESC) {
				terminate = true;
				exitGame = true;
				break;
			}
			if (FsGetKeyState(FSKEY_UP)) {
				sy += 0.5 * -cos(theta * (MYPI / 180.0));
				sx += 0.5 * sin(theta * (MYPI / 180.0));
			}
			if (FsGetKeyState(FSKEY_LEFT)) {
				theta += 6.0;
				if (theta >= 360.0) theta -= 360.0;
			}
			if (FsGetKeyState(FSKEY_RIGHT)) {
				theta -= 6.0;
				if (theta < 0.0) theta += 360.0;
			}
			if (FsGetKeyState(FSKEY_D)) {
				if (playerFireCounterRight >= playerFireInterval) {
					cannonball.Fire(sx, sy, theta, 0);
					soundManager.PlayCannonFireSound();
					playerFireCounterRight = 0;
				}
			}
			if (FsGetKeyState(FSKEY_A)) {
				if (playerFireCounterLeft >= playerFireInterval) {
					cannonball.Fire(sx, sy, theta, 1);
					soundManager.PlayCannonFireSound();
					playerFireCounterLeft = 0;
				}
			}
			playerFireCounterLeft++;
			playerFireCounterRight++;
			if (sx > worldWidth)
				sx = 0.0;
			else if (sx < 0.0)
				sx = worldWidth;
			if (sy > worldHeight)
				sy = 0.0;
			else if (sy < 0.0)
				sy = worldHeight;
			enemyFireCounter++;
			if (enemyFireCounter >= enemyFireInterval) {
				for (const auto& ship : enemyLogic.enemies) {
					if (ship.alive) {
						if (ECupDown == true) {
							enemyCannonball.Fire(ship.x, ship.y, ship.angle);
							soundManager.PlayCannonFireSound();
							ECupDown = false;
						}
						else {
							enemyCannonball.Fire(ship.x, ship.y, -ship.angle);
							soundManager.PlayCannonFireSound();
							ECupDown = true;
						}
					}
				}
				enemyFireCounter = 0;
			}
			cannonball.CannonMove();
			cannonball.CannonRender();
			enemyCannonball.Move();
			enemyCannonball.Render();
			playerShip.DrawShip(sx, sy, theta, true);
			enemyLogic.UpdateAndRenderEnemies(cannonball, score, obstacles);
			for (int i = 0; i < nCan; ++i) {
				if (enemyCannonball.cState[i]) {
					double dx = enemyCannonball.cx[i] - sx;
					double dy = enemyCannonball.cy[i] - sy;
					double dist = sqrt(dx * dx + dy * dy);
					if (dist < 2.0) {
						soundManager.PlayCollisionSound();
						terminate = true;
						break;
					}
				}
			}
			if (terminate) break;
			for (const auto& obs : obstacles) {
				double dx = obs.x - sx;
				double dy = obs.y - sy;
				double dist = sqrt(dx * dx + dy * dy);
				if (dist < (obs.radius + 2.0)) {
					soundManager.PlayCollisionSound();
					terminate = true;
					break;
				}
			}
			if (terminate) break;
			for (const auto& obs : obstacles) {
				obs.DrawObstacle();
			}
			for (int i = 0; i < nCan; ++i) {
				if (cannonball.cState[i]) {
					for (auto& obs : obstacles) {
						double dx = cannonball.cx[i] - obs.x;
						double dy = cannonball.cy[i] - obs.y;
						double dist = sqrt(dx * dx + dy * dy);
						if (dist < (obs.radius + 0.5)) {
							cannonball.cState[i] = false;
						}
					}
				}
			}
			for (int i = 0; i < nCan; ++i) {
				if (enemyCannonball.cState[i]) {
					for (auto& obs : obstacles) {
						double dx = enemyCannonball.cx[i] - obs.x;
						double dy = enemyCannonball.cy[i] - obs.y;
						double dist = sqrt(dx * dx + dy * dy);
						if (dist < (obs.radius + 0.5)) {
							enemyCannonball.cState[i] = false;
						}
					}
				}
			}
			if (enemyLogic.AllEnemiesDestroyed()) {
				terminate = true;
			}
			if (enemyLogic.PlayerDefeated(sx, sy)) {
				terminate = true;
			}
			glColor3ub(255, 255, 255);
			std::string scoreText = "Score: " + std::to_string(score);
			glRasterPos2i(10, 30);
			YsGlDrawFontBitmap16x20(scoreText.c_str());
			soundManager.KeepPlaying();
			FsSwapBuffers();
			FsSleep(25);
		}
		uiManager.SaveScore(score);
		soundManager.StopGameMusic();
		bool victory = enemyLogic.AllEnemiesDestroyed();
		uiManager.ShowEndScreen(victory, score, soundManager);
		soundManager.PlayIntroMusic();
	}
	return 0;
}
