event = require(Event)
tween = require(Tween)

local Animate = {}
Animate.__index = Animate

function Animate.new()
  instance = {
	component = nil,
	loop = false,
        animationIndex,
        elapsedTime = 0
        startInfo = {}
        animations = []
  }
  setmetatable(instance, Animate)

  event.register("update", Animate, Animate.update)

  return instance
end

function Animate.add(time, tweenInfo)
  self.animations[#self.animations+1] = {duration=time, info=tweenInfo}
end

function Animate:start(component, loop)
  self.component = component
  self.loop = loop
  self.elapsedTime = 0
  self.startInfo = component:getProperties()

end

end

function Animate:update(dt)
  if self.animationIndex > #self.animations and not self.loop then
    return
  end  

  self.elapsedTime = self.elapsedTime + dt
  animation = self.animations[animationIndex]
 
  if animation.duration > self.elapsedTime then
    self.animationIndex = self.animationIndex + 1
    self.elapsedTime = 0
    self.startInfo = component:getProperties()

    if self.animationIndex > #self.animations and self.loop then
    	self.animationIndex = 0
    end
  end
  curr = {
	x = tween.calculate(animation.x, startInfo.x, duration, elapsedTime),
	y = tween.calculate(animation.y, startInfo.y, duration, elapsedTime),
	width = tween.calculate(animation.width, startInfo.width, duration, elapsedTime),
	height = tween.calculate(animation.height, startInfo.height, duration, elapsedTime),
	transparency = tween.calculate(animation.transparency, startInfo.transparency, duration, elapsedTime),
	rotate = Tween.calculate(animation.rotate, startInfo.rotate, duration, elapsedTime)
  }

  component.setPosition(componentId, curr.x, curr.y)
  component.setDimensions(componentId, curr.width, curr.height)
  component.setTransparency(componentId, curr.transparency)
  component.setRotate(componentId, curr.rotate)
end

return Animate
