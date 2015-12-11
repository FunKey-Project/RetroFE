Component = require("Component")
Event = require("Event")

local Image = {}
Image.__index = Image

function Image.new()
  instance = {id = 0}
  setmetatable(instance, Image)

  Event.register("draw", Image, Image.draw)
  return instance
end

function Image:load(name)
  self.id = image.load(name)
  self.originalWidth, self.originalHeight = image.getDimensions(self.id)
end

function Image:draw()
  image.draw(component.getProperties())
end

setmetatable(Image, {__index = Component})
return Image
