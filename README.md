# Flocking
Flocking game test in Unreal Engine 5.2.

In this game, there are yellow boids with steering behaviors (alignment, cohesion and separation) inside the encapsulated scene. These boids avoid colliding with obstacles and with the surrounding capsule so they cannot leave the scene.


https://github.com/Keryen/Flocking/assets/32041567/35de6da4-0ac3-469f-8b65-875c5b0f3591


The player can shoot red boids with the left mouse button that chase the yellow boids around to consume them, while yellow boids try to run away from them. These red boids bounce if they fit an obstacle using custom physics logic. After consuming several yellow boids, the red boid becomes a yellow boid over time and joins other yellow ball groups. If the red boid consumes all its energy without consuming a yellow boid, it simply disappears.


https://github.com/Keryen/Flocking/assets/32041567/6055a8c8-e939-4505-a627-c5e464f6c141


Boids are managed in memory by a pool that controls active and inactive balls. This pattern is used so that all accesses are efficient to improve performance in terms of make it possible to run the application smoothly even for 1000 boids at least at 30 fps.
