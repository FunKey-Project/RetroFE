local Component = {}
Component.__index = Component

function Component.new()
  instance = {
    x = 0,
    y = 0,
    width = 0,
    height = 0,
    transparency = 1,
    rotate = 0,
    originalWidth = 0,
    originalHeight = 0
  }
     
  setmetatable(instance, Image)
  return instance
end

function Component:getProperties()
  return self
end


