# Flocking
Flocking game test in Unreal Engine.

In this game, there are some yellow boids with steering behaviors (alignment, cohesion and separation) inside the encapsulated scene. These boids avoid colliding with the obstacles inside and with the capsule so they cannot get out from the scene.

The player can shoot red boids that pursue those yellow boids to consume them, while yellow boids try fleeing from them. These red boids bounce if they collide with any obstacle. When a red ball has consumed some yellow boids, it turns into a yellow boid over time and joins other yellow ball groups.
