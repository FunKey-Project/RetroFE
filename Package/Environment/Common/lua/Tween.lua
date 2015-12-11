local Tween = {}
Tween.__index = Tween

function Tween.new()
  instance = {
	tweens = {}
	}

  setmetatable(instance, Tween)

  instance.tweens = {
    easeInQuadratic = Tween.easeInQuadratic,
    easeOutQuadratic = Tween.easeOutQuadratic,
    easeInOutQuadratic = Tween.easeInOutQuadratic,
    easeInCubic = Tween.easeInCubic,
    easeOutCubic = Tween.easeOutCubic,
    easeInOutCubic = Tween.easeInOutCubic,
    easeInQuartic = Tween.easeInQuartic,
    easeOutQuartic = Tween.easeOutQuartic,
    easeInOutQuartic = Tween.easeInOutQuartic,
    easeInQuintic = Tween.easeInQuintic,
    easeOutQuintic = Tween.easeOutQuintic,
    easeInOutQuintic = Tween.easeInOutQuintic,
    easeInSine = Tween.easeInSinev,
    easeOutSine = Tween.easeOutSine,
    easeInOutSine = Tween.easeInOutSine,
    easeInExponential = Tween.easeInExponential,
    easeOutExponential = Tween.easeOutExponential,
    easeInOutExponential = Tween.easeInOutExponential,
    easeInCircular = Tween.easeInCircular,
    easeOutCircular = Tween.easeOutCircular,
    easeInOutCircular = Tween.easeInOutCircular,
    linear = Tween.linear
  }

  return instance
end

function Tween.calculate(type, startval, endval, duration, elapsedTime)
    a = startval;
    b = endval - startval;
    result = tweens[type](elapsedTime, duration, a, b)
end


function Tween.linear(t, d, b, c)
    if d == 0 then 
        return b 
    end

    return c*t/d + b
end

function Tween.easeInQuadratic(t, d, b, c)
    if d == 0 then 
        return b 
    end

    t = t/d
    return c*t*t + b
end

function Tween.easeOutQuadratic(t, d, b, c)
    if d == 0 then 
        return b 
    end

    t = t/d
    return -1 * c * t*(t-2) + b
end

function Tween.easeInOutQuadratic(t, d, b, c)
    if d == 0 then 
        return b 
    end

    t = t/(d/2)
    if (t < 1) then 
	return c/2*t*t + b 
    end
    t = t - 1
    return -1 * c/2 * (t*(t-2) - 1) + b
end

function Tween.easeInCubic(t, d, b, c)
    if d == 0 then 
        return b 
    end

    t = t/d
    return c*t*t*t + b
end

function Tween.easeOutCubic(t, d, b, c)
    if d == 0 then 
        return b 
    end

    t = t/d
    t = t - 1
    return c*(t*t*t + 1) + b
end

function Tween.easeInOutCubic(t, d, b, c)
    if d == 0 then 
        return b 
    end

    t = t/(d/2)
    if t < 1 then
        return c/2*t*t*t + b
    end

    t = t - 2
    return c/2*(t*t*t + 2) + b
end

function Tween.easeInQuartic(t, d, b, c)
    if d == 0 then 
        return b 
    end

    t = t/d
    return c*t*t*t*t + b
end

function easeOutQuartic(t, d, b, c)
    if d == 0 then 
        return b 
    end

    t = t/d
    t = t - 1
    return -1 * c * (t*t*t*t - 1) + b
end

function Tween.easeInOutQuartic(t, d, b, c)
    if d == 0 then 
        return b 
    end

    t = t/(d/2)
    if t < 1 then
        return c/2*t*t*t*t + b
    end

    t = t - 2
    return -1 * c/2 * (t*t*t*t - 2) + b
end

function Tween.easeInQuintic(t, d, b, c)
    if d == 0 then 
        return b 
    end

    t = t/d
    return c*t*t*t*t*t + b
end


function Tween.easeOutQuintic(t, d, b, c)
    if d == 0 then return b end
    t = t/d
    t = t - 1
    return c*(t*t*t*t*t + 1) + b
end

function Tween.easeInOutQuintic(t, d, b, c)
    if d == 0 then 
        return b 
    end

    t = t / (d/2)
    if t < 1 then
        return c/2*t*t*t*t*t + b
    end

    t = t - 2
    return c/2*(t*t*t*t*t + 2) + b
end

function Tween.easeInSine(t, d, b, c)
    return -1 * c * cos(t/d * (math.pi / 2)) + c + b
end

function Tween.easeOutSine(t, d, b, c)
    return c * math.sin(t/d * (math.pi / 2)) + b
end

function Tween.easeInOutSine(t, d, b, c)
    return -1 * c/2 * (math.cos( math.pi * t/d) - 1) + b
end

function Tween.easeInExponential(t, d, b, c)
    return c * math.pow( 2, 10 * (t/d - 1) ) + b
end

function Tween.easeOutExponential(t, d, b, c)
    return c * ( - math.pow( 2, -10 * t/d ) + 1 ) + b
end

function Tween.easeInOutExponential(t, d, b, c)
    t = t/(d/2)
    if (t < 1) return c/2 * math.pow( 2, 10 * (t - 1) ) + b
    t = t - 1
    return c/2 * ( -1* math.pow( 2, -10 * t) + 2 ) + b
end

function Tween.easeInCircular(t, d, b, c)
    t = t/d
    return -1 * c * (sqrt(1 - t*t) - 1) + b
end


function Tween.easeOutCircular(t, d, b, c)
    t = t/d
    t = t - 1
    return c * sqrt(1 - t*t) + b
end

function Tween.easeInOutCircular(t, d, b, c)
    t = t/(d/2)
    if (t < 1) return -c/2 * (sqrt(1 - t*t) - 1) + b
    t = t - 2
    return c/2 * (sqrt(1 - t*t) + 1) + b
end



