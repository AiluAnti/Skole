import pygame
import math
import random



pygame.init()


#defining some colors
black = (0, 0, 0)
white = (255, 255, 255)
blue = (0, 0, 255)
red = (208, 0, 0)

#Defining some fonts that will come in handy
score_font = pygame.font.Font(None, 25)
lives_font = pygame.font.Font(None, 25)
game_over_font = pygame.font.Font(None, 30)
win_text_font = pygame.font.Font(None, 30)
you_won_font = pygame.font.Font(None, 30)

#Defining some global variables that will be used later
score = 0
lives = 3
level = 1

paddle_width = 70
paddle_height = 10

brick_width = 40
brick_height = math.floor(brick_width / 2)

#Creating the spritelists
brick_list = pygame.sprite.Group()
ball_list = pygame.sprite.Group()
paddle_list = pygame.sprite.Group()

class Canvas():
    def __init__(self):

        #Setting up the screen
        pygame.display.set_mode((640, 480))
        pygame.display.set_caption("Breakout")
        self.screen = pygame.display.get_surface()

        # Get attributes for the height/width of the screen
        self.screenheight = pygame.display.get_surface().get_height()
        self.screenwidth = pygame.display.get_surface().get_width()
        self.screen.fill(black)

        #Creating the clock object
        self.clock = pygame.time.Clock()

        self.construct_level()

    def update(self):
        #Setting the framerate
        self.clock.tick(200)

        #Fetching some global variables to be used in this method
        global score
        global level
        global brick_width
        global brick_height
        global paddle_width
        global paddle_height

        if (level > 4 and lives > 0):
            you_won = you_won_font.render("Congratulations! You beat the game with a score of: %4d" %score, False, (255, 255, 255))
            self.screen.blit(you_won, (self.screenwidth / 2 - you_won.get_rect().centerx, self.screenheight / 2 - 10))

        #Congratulation message if the player manages to remove all the bricks
        elif (len(brick_list.sprites()) <= 0 and lives > 0 and level <= 3):
            win_text = win_text_font.render("Congratulations! You beat level %d"%level, False, (255, 255, 255))
            #Writing in in the centre of the screen
            self.screen.blit(win_text, (self.screenwidth / 2 - win_text.get_rect().centerx, self.screenheight / 2))

        #Write "Game Over" if you have no lives left
        elif (lives <= 0):
            game_over = game_over_font.render("Game Over, press 'Enter' to play again.", False, (255, 255, 255))
            self.screen.blit(game_over, (self.screenwidth / 2 - game_over.get_rect().centerx, self.screenheight / 2))

        #If neither of the above, continue running the game
        else:
            #This is what will be updated every tick
            paddle_list.update()
            ball_list.update()
            self.screen.fill((0, 0, 0))
            paddle_list.draw(self.screen)
            brick_list.draw(self.screen)
            ball_list.draw(self.screen)

            #Display lives left
            lives_left = lives_font.render("Lives: %02d"% lives, False, (255, 255, 255))
            self.screen.blit(lives_left, (self.screenwidth - lives_left.get_rect().width, 0))

            #Display player score
            score_text = score_font.render("Score: %04d"% score, False, (255, 255, 255))
            self.screen.blit(score_text, (0, 0))

            #Display what level the player is on
            current_level = score_font.render("Level: %02d"% level, False, (208, 255, 255))
            self.screen.blit(current_level, (self.screenwidth / 2 - current_level.get_rect().centerx, 0))

        pygame.display.flip()


    def handle_events(self):
        """This method handles all the relevant events for this game"""

        #Fetching pygame events 
        for event in pygame.event.get():
            keys = {pygame.K_RIGHT: 1,
                    pygame.K_LEFT: -1,
                    pygame.K_a: -1,
                    pygame.K_d: 1}

            if event.type ==  pygame.KEYUP:
                if event.key in keys:
                    for paddle in paddle_list:
                        paddle.direction = 0

            elif event.type == pygame.KEYDOWN:
                if event.key in keys:
                    for paddle in paddle_list:
                        paddle.direction = keys[event.key]

            if (lives <= 0):
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_RETURN:
                        self.remove_sprites()
                        self.construct_level()

            elif (lives > 0 and len(brick_list.sprites()) <= 0):
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_RETURN:
                        self.remove_sprites()
                        self.new_level()

            if event.type == pygame.QUIT:
                exit()

    def remove_sprites(self):
        """This method removes all the sprites from the screen"""

        ball_list.empty()
        paddle_list.empty()
        brick_list.empty()

    def construct_level(self):
        """This method creates the first level to be played"""

        global lives
        global score
        global level

        lives = 3
        score = 0
        level = 1

        #number of lines with bricks
        num_lines = 7
        #defining how many bricks go in one row
        
        #brick_line = math.floor(self.screenwidth/brick_width) - 1
        brick_line = math.floor((self.screenwidth - 80) / brick_width)

        #for loop creating the bricks
        for i in range(brick_line * num_lines):
            #the line variable tells the program what line to draw the brick on
            self.line = math.floor(i/brick_line)
            #the brick_x variable makes it impossible for bricks to be created outside the screen by using modulus
            self.brick_x = i % brick_line * brick_width + i % brick_line
            #creating the bricks with given x position explained further up.
            #brick_height * line + line makes it so that we stay on the same y value for i = 0 -> i = 22
            brick_list.add(Brick(brick_width + self.brick_x, brick_height * self.line + self.line))

        paddle_list.add(Paddle(self.screenwidth / 2 - paddle_width / 2, self.screenheight - self.screenheight / 20))

        ball_list.add(Ball(self.screenwidth / 2, self.screenheight / 2))

    def new_level(self):
        """This method creates all levels past the first one"""
        #Fetching some global variables
        global level
        global lives
        global score

        lives = 3
        level += 1

        #number of lines with bricks
        num_lines = 7 + level

        #defining how many bricks go in one row
        
        #brick_line = math.floor(self.screenwidth/brick_width) - 1
        brick_line = math.floor((self.screenwidth - 80) / brick_width)

        #for loop creating the bricks
        for i in range(brick_line * num_lines):
            #the line variable tells the program what line to draw the brick on
            self.line = math.floor(i/brick_line)
            #the brick_x variable makes it impossible for bricks to be created outside the screen by using modulus
            self.brick_x = i % brick_line * brick_width + i % brick_line
            #creating the bricks with given x position explained further up.
            #brick_height * line + line makes it so that we stay on the same y value for i = 0 -> i = 22
            brick_list.add(Brick(brick_width + self.brick_x, brick_height * self.line + self.line))

        paddle_list.add(Paddle(self.screenwidth / 2 - paddle_width / 2, self.screenheight - self.screenheight / 20))

        ball_list.add(Ball(self.screenwidth / 2, self.screenheight / 2))

    def run(self):
        # The main loop.
        while 1:
            self.handle_events()
            self.update()


class Paddle(pygame.sprite.Sprite):
    def __init__(self, x, y):

        #initiate parentclass(sprite)
        super().__init__()

        #creating the paddle
        self.image = pygame.Surface([paddle_width, paddle_height])
        #filling it with a color
        self.image.fill(white)
        self.rect = self.image.get_rect()
        self.rect.x = x
        self.rect.y = y
        self.direction = 0

    def update(self):
        """here we update the position of the paddle"""
        self.rect.x += self.direction * pygame.display.get_surface().get_width() / 200
        #making the paddle unable to move beyond the borders of the screen
        if (self.rect.x + paddle_width >= pygame.display.get_surface().get_width()):
            self.direction = 0
        elif (self.rect.x <= 0):
            self.direction = 0


class Ball(pygame.sprite.Sprite):
    def __init__(self, x, y):

        #initiate parent class
        super().__init__()
        global level

        self.ball_img = pygame.image.load('ball.png').convert_alpha()
        pygame.display.get_surface().blit(self.ball_img, (200, 200))

        #creating ball
        self.image = self.ball_img
        self.rect = self.image.get_rect()
        self.rect.x = x
        self.rect.y = y
        self.x = x
        self.y = y
        self.direction = 0
        self.speed_x = 0
        self.speed_y = 2 + math.floor(level / 2)


    def check_collision(self, rect):
        if (rect.collidepoint(self.rect.midbottom)):
            return "bottom"
        if (rect.collidepoint(self.rect.midleft)):
            return "left"
        if (rect.collidepoint(self.rect.midtop)):
            return "top"
        if (rect.collidepoint(self.rect.midright)):
            return "right"
        return None


    def update(self):
        self.y += self.speed_y
        self.x += self.speed_x
        self.rect.y = self.y
        self.rect.x = self.x
        global lives

        if (self.rect.x + self.speed_x + 10 >= pygame.display.get_surface().get_width()):
            self.speed_x = -self.speed_x

        elif (self.rect.x + self.speed_x <= 0):
            self.speed_x = -self.speed_x

        if (self.rect.y + self.speed_y + 10 >= pygame.display.get_surface().get_height()):
            lives = lives - 1
            ball_list.remove(self)
            ball_list.add(Ball(pygame.display.get_surface().get_width() / 2, pygame.display.get_surface().get_height() / 2))
            
        elif (self.rect.y + self.speed_y < 0):
            self.speed_y = -self.speed_y


        ball_paddle_collision = pygame.sprite.groupcollide(ball_list, paddle_list, False, False)

        if ball_paddle_collision:
            self.speed_x = (self.rect.centerx - ball_paddle_collision[self][0].rect.centerx) / (paddle_width / 2)
            self.speed_y = -self.speed_y

        ball_brick_collision = pygame.sprite.groupcollide(ball_list, brick_list, False, True)

        if ball_brick_collision:
            for Brick in ball_brick_collision:
                global score
                score += 10

            col_point = self.check_collision(ball_brick_collision[self][0].rect)

            if col_point in ("left", "right"):
                self.speed_x = -self.speed_x
            elif col_point in ("top", "bottom"):
                self.speed_y = -self.speed_y
            else:
                self.speed_y = -self.speed_y


class Brick(pygame.sprite.Sprite):
    def __init__(self, x, y):
        #initiate parentclass(sprite)
        super().__init__()
        #set the properties of the brick
        self.image = pygame.Surface([brick_width, brick_height])
        #fill it with a color
        self.image.fill((random.randrange(0, 255), random.randrange(0, 255), random.randrange(0, 255)))
        self.rect = self.image.get_rect()
        self.rect.x = x
        self.rect.y = y

if __name__ == "__main__":
    cv = Canvas()
    cv.run()