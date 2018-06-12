import pygame

SCREEN_X = 1080
SCREEN_Y = 720

FONT_INIT 	 = pygame.font.init()
PLAYER1_FONT = pygame.font.SysFont("comicsansms", 25)
WHITE = (255 ,255, 255)
RED = (255, 0, 0)

ANGLE_DIR = 15

FUEL_MAX = 100.0
FUEL_LOSS = float(1.0/20.0)
HEALTH_MAX = 100

GRAVITY = 1
FPS = 200
ORIGIN = (0, 0)
DIR_MAX = 360
VELOCITY_MAX = 15
BULLET_MAX = 20