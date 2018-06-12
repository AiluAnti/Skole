import pygame
import math
import random
from precode import Vector2D

pygame.init()

black = (0, 0, 0)

class Canvas():
    def __init__(self):
        #Setting up the screen
        pygame.display.set_mode((1080,720))
        pygame.display.set_caption("Boids")
        self.screen = pygame.display.get_surface()

        # Get attributes for the height/width of the screen
        self.screenheight = pygame.display.get_surface().get_height()
        self.screenwidth = pygame.display.get_surface().get_width()
        self.screen.fill(black)

        self.boid = Boids(random.randrange(1, self.screenwidth), random.randrange(1, self.screenheight))
        self.hoik = Hoiks(random.randrange(1, self.screenwidth), random.randrange(1, self.screenheight))
        self.obstacle = Obstacles(random.randrange(1, self.screenwidth), random.randrange(1, self.screenheight))
        #creating the clock object
        self.clock = pygame.time.Clock()
        for i in range (1, 30):
            self.boid.boid_list.add(Boids(self.screenwidth + random.randrange(1,100), self.screenheight + random.randrange(1, 100)))
        for i in range (1, 4):
            self.hoik.hoik_list.add(Hoiks(random.randrange(1, self.screenwidth), random.randrange(1, self.screenheight)))
        for i in range (1, 4):
            self.obstacle.obstacle_list.add(Obstacles(random.randrange(1, self.screenwidth), random.randrange(1, self.screenheight)))

    def update(self):
        #setting the framerate
        self.clock.tick(30)
        self.boid.update(self.hoik.hoik_list, self.obstacle.obstacle_list)
        self.hoik.update()
        self.obstacle.update()
        self.screen.fill((255, 255, 255))
        self.obstacle.obstacle_list.draw(self.screen)
        self.boid.boid_list.draw(self.screen)
        self.hoik.hoik_list.draw(self.screen)
        pygame.display.flip()

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_1:
                    self.boid.boid_list.add(Boids(random.randrange(1, self.screenwidth), random.randrange(1, self.screenheight)))

            if event.type == pygame.QUIT:
                exit()

    def run(self):
        while 1:
            self.handle_events()
            self.update()


class Boids(pygame.sprite.Sprite):
    def __init__(self, x, y):
        super().__init__()

        self.boid_img = pygame.image.load('boid.png').convert_alpha()
        pygame.display.get_surface().blit(self.boid_img, (x, y))

        self.image = self.boid_img
        #self.image.fill((random.randrange(10, 255), random.randrange(10, 255), random.randrange(10, 255)))
        self.rect = self.image.get_rect()
        self.rect.x = x
        self.rect.y = y
        self.boid_list = pygame.sprite.Group()
        self.position = Vector2D(self.rect.x, self.rect.y)
        self.velocity = Vector2D(0,0)
        self.radius = self.rect.x - self.rect.centerx

    def update(self, hoiks, obstacles):
        for boid in self.boid_list:
            v1 = boid.first_rule(self.boid_list) 
            v2 = boid.second_rule(self.boid_list)
            v3 = boid.third_rule(self.boid_list)
            v4 = boid.fourth_rule(self.boid_list, hoiks)
            v5 = boid.avoid_obstacles(self.boid_list, obstacles)
            boid.velocity += v1 + v2 + v3 + v4
            boid.limit_velocity(self.boid_list)
            boid.position += boid.velocity
            boid.rect.x += boid.velocity.x
            boid.rect.y += boid.velocity.y      

            if (boid.rect.x > pygame.display.get_surface().get_width()):
                boid.rect.x = 0
            elif (boid.rect.x < 0):
                boid.rect.x = pygame.display.get_surface().get_width()
            if (boid.rect.y > pygame.display.get_surface().get_height()):
                boid.rect.y = 0
            elif (boid.rect.y < 0):
                boid.rect.y = pygame.display.get_surface().get_height()

    def limit_velocity(self, liste):
        max_velocity = 20
        for boid in liste:
            if (boid.velocity.magnitude() > max_velocity):
                boid.velocity *= (1.0 /boid.velocity.magnitude() / max_velocity) * max_velocity

    def first_rule(self, liste):
        c = Vector2D(0,0)
        a = 0
        for boid in liste:
            if boid is not self:
                distance = (boid.position - self.position).magnitude()
                if distance < 500:
                    c += boid.position
                    a += 1
        if a > 0:      
            c *= (1.0/a)
            c = ((c - self.position) * 0.01)
        return c * 0.9

    def second_rule(self, liste):
        c = Vector2D(0, 0)
        for boid in liste:
            boid.radius = boid.rect.centerx - boid.rect.x
            if (boid.position - self.position).magnitude() < boid.radius * 5:
                c -= (boid.position - self.position)
        return c * 0.2

    def third_rule(self, liste):
        speedvector = Vector2D(0, 0)
        a = 0
        for boid in liste:
            if boid is not self:
                distance = (boid.position - self.position).magnitude()
                if distance < 400:
                    speedvector += boid.velocity
                    a += 1
        if (a > 0):
            speedvector = speedvector * (1 / a)

        return (speedvector - (self.velocity)) * (1/2)

    def fourth_rule(self, liste, hoiks):
        c = Vector2D(0, 0)
        for boid in liste:
            boid.radius = boid.rect.centerx - boid.rect.x
            for hoik in hoiks:
                if (boid.position - hoik.position).magnitude() < 200:
                    c -= (boid.position - hoik.position)
        return c * 0.3

    def avoid_obstacles(self, liste, obstacles):
        c = Vector2D(0, 0)
        for boid in liste:
            boid.radius = boid.rect.centerx - boid.rect.x
            for obstacle in obstacles:
                if (boid.position - obstacle.position).magnitude() < 300:
                    c -= (boid.position - obstacle.position)
        return c


class Hoiks(pygame.sprite.Sprite):
    def __init__(self, x, y):
        super().__init__()
        self.hoik_img = pygame.image.load('hoik.png').convert_alpha()
        pygame.display.get_surface().blit(self.hoik_img, (x, y))

        self.image = self.hoik_img
        self.rect = self.image.get_rect()
        self.rect.x = x
        self.rect.y = y
        self.hoik_list = pygame.sprite.Group()
        self.position = Vector2D(self.rect.x, self.rect.y)
        self.velocity = Vector2D(0,0)
        self.radius = self.rect.x - self.rect.centerx

    def update(self):
        for hoik in self.hoik_list:
            v1 = Vector2D(4, 0)
            hoik.velocity += v1
            hoik.limit_velocity(self.hoik_list)
            hoik.rect.x += hoik.velocity.x
            hoik.rect.y += hoik.velocity.y
            hoik.position.y = hoik.rect.y
            hoik.position.x = hoik.rect.x

            if (hoik.rect.x > pygame.display.get_surface().get_width()):
                hoik.rect.x = 0
            elif (hoik.rect.x < 0):
                hoik.rect.x = pygame.display.get_surface().get_width()
            if (hoik.rect.y > pygame.display.get_surface().get_height()):
                hoik.rect.y = 0
            elif (hoik.rect.y < 0):
                hoik.rect.y = pygame.display.get_surface().get_height()

    def limit_velocity(self, liste):
        max_velocity = 15
        for hoik in liste:
            if (hoik.velocity.magnitude() > max_velocity):
                hoik.velocity *= (1.0 /hoik.velocity.magnitude() / max_velocity) * max_velocity


class Obstacles(pygame.sprite.Sprite):
    def __init__(self, x, y):
        super().__init__()
        self.obstacle_img = pygame.image.load('obstacle.png').convert_alpha()
        pygame.display.get_surface().blit(self.obstacle_img, (x, y))

        self.image = self.obstacle_img
        self.rect = self.image.get_rect()
        self.rect.x = x
        self.rect.y = y
        self.obstacle_list = pygame.sprite.Group()
        self.position = Vector2D(self.rect.x, self.rect.y)

if __name__ == "__main__":
    cv = Canvas()
    cv.run()
