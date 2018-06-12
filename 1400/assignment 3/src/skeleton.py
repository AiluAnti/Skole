from precode import Vector2D
from config import *
import pygame
from math import cos, sin, degrees, radians
import random


class Game():
	"""The class controlling the game, and updating all the functions"""
	def __init__(self):
		"""Initializing the Game class"""
		pygame.init() #initializing pygame
		pygame.display.set_mode((SCREEN_X, SCREEN_Y)) #setting screen size
		pygame.display.set_caption("skeleton")
		self.screen 	= pygame.display.get_surface() 
		self.background = pygame.image.load('pictures/background.png').convert_alpha() #loading the background image and converting it
		self.background = pygame.transform.scale(self.background, (SCREEN_X, SCREEN_Y)) #making the background image fit the set screen size

		self.clock 		= pygame.time.Clock() #initializing the pygame clock object
		FONT_INIT #initializing pygame fonts

		#adding the needed objects to their respective groups
		Spaceship.list.add(Spaceship(SCREEN_X / 2 - SCREEN_X / 4, SCREEN_Y / 2, 'pictures/spaceships/player.png'))
		Spaceship.list.add(Spaceship(SCREEN_X / 2 + SCREEN_X / 4, SCREEN_Y / 2, 'pictures/spaceships/player2.png'))
		Obstacle.list.add(Obstacle(SCREEN_X / 2, SCREEN_Y / 2))
		Fuel.list.add(Fuel(random.randrange(0, SCREEN_X - 50), random.randrange(20, SCREEN_Y - 50)))

		self.ticker 	= 0 
		self.ticker_2 	= 0
		self.run() #running the Game class
	def update(self):
		"""The update method of the class. This method makes sure all the sprite groups get drawn to the screen, and all the text as well."""
		self.clock.tick(FPS) #setting the frame rate

		#updating all the groups containing objects
		Bullet.list1.update()
		Bullet.list2.update()
		Spaceship.list.update()
		Obstacle.list.update()
		Fuel.list.update()
		pygame.display.get_surface().blit(self.background, ORIGIN) #blitting the background at (0,0)

		if len(Fuel.list) < 1:
			Fuel.list.add(Fuel(random.randrange(20, SCREEN_X - 20), random.randrange(20, SCREEN_Y - 20))) #if fuel gets picked up, replenish with another one

		if len(Spaceship.list) < 2:
			Restart() #if a player "dies", reset the game and all the stats
		
		if len(Spaceship.list) > 1: #the game is only supposed to run as long as both players are alive
			#display score, health, and fuel status of both players
			space_health = PLAYER1_FONT.render("Health player 2: %3d" %Spaceship.list.sprites()[0].health, False, WHITE)
			self.screen.blit(space_health, (SCREEN_X / 2 + SCREEN_X / 4 - space_health.get_rect().centerx, 20))
			
			space_health2 = PLAYER1_FONT.render("Health player 1: %3d" %Spaceship.list.sprites()[1].health, False, WHITE)
			self.screen.blit(space_health2, (SCREEN_X / 4 - space_health2.get_rect().centerx, 20))

			player1_score = PLAYER1_FONT.render("Score player 2: %3d" %Spaceship.list.sprites()[0].score, False, WHITE)
			self.screen.blit(player1_score, (SCREEN_X / 2 + SCREEN_X / 4 - space_health.get_rect().centerx, 50))

			player2_score = PLAYER1_FONT.render("Score player 1: %3d" %Spaceship.list.sprites()[1].score, False, WHITE)
			self.screen.blit(player2_score, (SCREEN_X / 4 - space_health.get_rect().centerx, 50))

			player1_fuel = PLAYER1_FONT.render("Fuel player 2: %3d" %Spaceship.list.sprites()[0].fuel, False, WHITE)
			self.screen.blit(player1_fuel, (SCREEN_X / 2 + SCREEN_X / 4 - space_health.get_rect().centerx, 80))

			player2_fuel = PLAYER1_FONT.render("Fuel player 1: %3d" %Spaceship.list.sprites()[1].fuel, False, WHITE)
			self.screen.blit(player2_fuel, (SCREEN_X / 4 - space_health.get_rect().centerx, 80))

		#draw all the objects to the screen
		Bullet.list1.draw(self.screen)
		Bullet.list2.draw(self.screen)
		Spaceship.list.draw(self.screen)
		Obstacle.list.draw(self.screen)
		Fuel.list.draw(self.screen)
		pygame.display.flip()

		#restart the tickers if the reach their preset maximum
		if self.ticker_2 >= 6:
			self.ticker_2 = 0
		else:
			self.ticker_2 +=1

		if self.ticker >= 3:
			self.ticker = 0
		else:
			self.ticker +=1

	def handle_events(self):
		"""This method handles all keyboard input, most relevant being the direction controls as well as firing bullets"""
		for event in pygame.event.get():
			if event.type == pygame.QUIT:
				exit()
		keys = pygame.key.get_pressed()
		if self.ticker >= 3:
			if len(Spaceship.list) > 1: #the game is only supposed to run as long as both players are alive
				#registering all keyboard input, making sure the spaceships turn a given number of degrees. 
				#They also have the ability to thrust in the direction they are turned
				if keys[pygame.K_LEFT]:
					Spaceship.list.sprites()[0].dir 		+= ANGLE_DIR
				if keys[pygame.K_RIGHT]:
					Spaceship.list.sprites()[0].dir 		-= ANGLE_DIR
				if keys[pygame.K_UP]:
					Spaceship.list.sprites()[0].velocity.x 	+= (degrees(sin(radians(Spaceship.list.sprites()[0].dir)))) * -1
					Spaceship.list.sprites()[0].velocity.y 	+= (degrees(cos(radians(Spaceship.list.sprites()[0].dir)))) * -1
				if keys[pygame.K_p]:
					if self.ticker_2 >= 6:
						Bullet.list1.add(Bullet(Spaceship.list.sprites()[0].rect.centerx, Spaceship.list.sprites()[0].rect.centery, 0))
				if keys[pygame.K_a]:
					Spaceship.list.sprites()[1].dir 		+= ANGLE_DIR
				if keys[pygame.K_d]:
					Spaceship.list.sprites()[1].dir 		-= ANGLE_DIR
				if keys[pygame.K_w]:
					Spaceship.list.sprites()[1].velocity.x 	+= (degrees(sin(radians(Spaceship.list.sprites()[1].dir)))) * -1
					Spaceship.list.sprites()[1].velocity.y 	+= (degrees(cos(radians(Spaceship.list.sprites()[1].dir)))) * -1
				if keys[pygame.K_SPACE]:
					if self.ticker_2 >= 6:
						Bullet.list2.add(Bullet(Spaceship.list.sprites()[1].rect.centerx, Spaceship.list.sprites()[1].rect.centery, 1))
	def run(self):
		"""This method runs the game"""
		while 1:
			self.update()
			self.handle_events()

class Restart(pygame.sprite.Sprite):
	"""This class resets the game when a player dies or runs out of fuel"""
	def __init__(self):
		"""Initializing the Restart class"""
		super(Restart, self).__init__()
		if len(Spaceship.list) <= 1:
			#removing all visible objects before starting the game over
			for player in Spaceship.list:
				Spaceship.list.remove(player)
			for bullet1 in Bullet.list1:
				Bullet.list1.remove(bullet1)
			for bullet2 in Bullet.list2:
				Bullet.list2.remove(bullet2)
			for hindring in Obstacle.list:
				Obstacle.list.remove(hindring)
			for bensin in Fuel.list:
				Fuel.list.remove(bensin)
			Game()

class Fuel(pygame.sprite.Sprite):
	"""In this class we'll find the properties of the fuel object"""
	list = pygame.sprite.Group()
	def __init__(self, x, y): #I found it easier to take x and y as arguments instead of declaring them in the class.
		"""Initializing the Fuel class"""
		super(Fuel, self).__init__()

		self.img 			= pygame.image.load('pictures/fuel.png') #loading the needed image
		pygame.display.get_surface().blit(self.img, (x, y))

		self.image 			= self.img
		self.rect 			= self.image.get_rect()
		self.rect.x  		= x
		self.rect.y 		= y



class Spaceship(pygame.sprite.Sprite):
	"""This is the Spaceship-class, or the player-class. In this class, we'll find the properties of the Spaceship object"""
	list = pygame.sprite.Group()

	def __init__(self, x, y, picture): #the picture argument is added so that I can load different images for each spaceship.
		"""Initializing the Spaceship class"""
		super(Spaceship, self).__init__()

		self.img 			= pygame.image.load(picture).convert_alpha() #loading the given spaceship image
		self.img 			= pygame.transform.scale(self.img, (70, 70)) #scaling the picture so that any input image will be this size
		pygame.display.get_surface().blit(self.img, (x, y)) #blit the picture to the surface

		self.image 			= self.img
		self.rect 			= self.image.get_rect()
		self.rect.x 		= x
		self.rect.y 		= y
		self.rect.center 	= (x, y)
		self.dir 			= 0
		self.velocity 		= Vector2D(0, 0)
		self.score			= 0
		self.health 		= HEALTH_MAX
		self.fuel 			= FUEL_MAX
		

	def update(self):
		"""This is the update function of the class. For each passed frame, the position of the player will be updated, as well as their fuel."""
		self.collision()
		if self.fuel > FUEL_MAX: #don't let the player have more than FUEL_MAX amount of fuel
			self.fuel = FUEL_MAX
		elif self.fuel <= 0: 
			self.list.remove(self) #if fuel is empty, remove player from group; this will reset the game
		self.limit_velocity()

		self.fuel 			-= FUEL_LOSS #the players loses this amount of fuel for each passed frame
		self.rect.x 		+= self.velocity.x * 0.15
		self.rect.y 		+= self.velocity.y * 0.15 + GRAVITY
		#these four lines rotate the image
		old_center 			= self.rect.center

		self.image 			= pygame.transform.rotate(self.img, self.dir)
		
		self.rect 			= self.image.get_rect()

		self.rect.center 	= old_center

		#wallcollision. If a spaceship reaches the edge of he scereen, it will appear on the opposite edge
		if (self.rect.x >= SCREEN_X):
			self.rect.x = 1
		elif (self.rect.x + self.rect.width <= 0):
			self.rect.x = SCREEN_X - 1

		if (self.rect.y >= SCREEN_Y):
			self.rect.y = 1
		elif (self.rect.y + self.rect.height <= 0):
			self.rect.y = SCREEN_Y - 1

		#here, we assign a max value for how many degrees a spaceship can turn before being assigned back to 0 degrees
		if self.dir >= DIR_MAX:
			self.dir -= DIR_MAX
		elif self.dir <= 0:
			self.dir += DIR_MAX


	def limit_velocity(self):
		"""This method is made to make sure that a spaceship can't acquire infinite velocity"""
		if (self.velocity.magnitude() > VELOCITY_MAX):
			self.velocity = self.velocity.normalized() * VELOCITY_MAX #making sure the spaceship doesn't acquire more velocity than the set maximum

	def collision(self):
		"""Collision detection between spaceship and fuel"""
		player_collision  = pygame.sprite.groupcollide(self.list, Fuel.list, False, True)
		for player in player_collision:
			self.fuel += 50 #add fuel to the player who reaches the fuel object on the screen


class Bullet(pygame.sprite.Sprite):
	"""This is the class from which the bullet objects derive"""
	list1 = pygame.sprite.Group() #making a group of bullets for each player
	list2 = pygame.sprite.Group() #this is to differ between who fires which bullets in the collision detections

	def __init__(self, x, y, num):
		"""Initializing the Bullet class"""
		super(Bullet, self).__init__()

		self.img 			= pygame.image.load('pictures/boid2.png').convert_alpha() #load image
		pygame.display.get_surface().blit(self.img, (x, y)) #blit image

		self.image 			= self.img
		if(num == 1):
			self.image.fill(RED) #making sure that both players don't have bullets that look exactly the same, with color being the only difference.

		self.image 			= pygame.transform.rotate(self.img, Spaceship.list.sprites()[num].dir) #this line of code shoots the bullets in the direction the given spaceship is pointed
		self.rect 			= self.image.get_rect()
		self.rect.x 		= x
		self.rect.y 		= y
		self.velocity 		= Vector2D((degrees(sin(radians(Spaceship.list.sprites()[num].dir)))) * -1, (degrees(cos(radians(Spaceship.list.sprites()[num].dir)))) * -1)

	def update(self):
		"""In the bullets update method, we check for collision with screen edges, and update each bullets position"""
		self.space_collision()
		#going through both lists of bullets, and performing wall collision
		#if the bullets collide with the wall, they disappear and are removed from their respective group
		for bullet in self.list1:
			if bullet.rect.x >= SCREEN_X:
				bullet.list1.remove(bullet)
			elif bullet.rect.x < 0:
				bullet.list1.remove(bullet)

			if bullet.rect.y >= SCREEN_Y:
				bullet.list1.remove(bullet)
			elif bullet.rect.y < 0:
				bullet.list1.remove(bullet)
		for bullet in self.list2:
			if bullet.rect.x >= SCREEN_X:
				bullet.list2.remove(bullet)
			elif bullet.rect.x < 0:
				bullet.list2.remove(bullet)

			if bullet.rect.y >= SCREEN_Y:
				bullet.list2.remove(bullet)
			elif bullet.rect.y < 0:
				bullet.list2.remove(bullet)

		self.limit_velocity() #limit the velocity of the bullets
		self.rect.x += self.velocity.x * 0.3
		self.rect.y += self.velocity.y * 0.3

	def space_collision(self):
		"""Checks for collision between players(spaceships) and the bullets"""
		if len(Spaceship.list) > 1:
			#remove spaceship from group and reset if health <= 0, remove a given amount of health from spaceship if not
			if (pygame.sprite.spritecollide(Spaceship.list.sprites()[0], self.list2, True)):
				if Spaceship.list.sprites()[0].health <= 0:
					Spaceship.list.remove(Spaceship.list.sprites()[0])
				else:
					Spaceship.list.sprites()[0].health -= 10
					Spaceship.list.sprites()[1].score += 10
			elif (pygame.sprite.spritecollide(Spaceship.list.sprites()[1], self.list1, True)):
				if Spaceship.list.sprites()[1].health <= 0:
					Spaceship.list.remove(Spaceship.list.sprites()[1])
				else:
					Spaceship.list.sprites()[1].health -= 10
					Spaceship.list.sprites()[0].score += 10

	def limit_velocity(self):
		"""The bullets need a limited maximum velocity to make sure they don't gain infinite velocity"""
		if (self.velocity.magnitude() > BULLET_MAX):
			self.velocity = self.velocity.normalized() * BULLET_MAX #limit all the bullets' velocity to a given maximum

class Obstacle(pygame.sprite.Sprite):
	"""The class that let's us create an Obstacle object"""
	list = pygame.sprite.Group()
	def __init__(self, x, y):
		"""initiating the Obstacle class"""
		super(Obstacle, self).__init__()
		self.img 	= pygame.image.load('pictures/obstacle.png').convert_alpha() #loading the image and converting it
		self.image 	= self.img
		self.rect 	=  self.image.get_rect()
		self.rect.x = x
		self.rect.y = y

	def update(self):
		"""Checks for collision between bullets and players"""
		#removes or destroys anything that collide with the obstacle, be it bullets or spaceships.
		player_collision  = pygame.sprite.spritecollide(self, Spaceship.list, False)
		for hit in player_collision:
			hit.score = 0
		bullet1_collision = pygame.sprite.spritecollide(self, Bullet.list1, True)
		bullet2_collision = pygame.sprite.spritecollide(self, Bullet.list2, True)


if __name__ == "__main__":
	Game()
