#include "Battery.h"
#include "../ViewInfo.h"
#include "../../SDL.h"
#include "../../Utility/Log.h"
#include <SDL/SDL_image.h>


#define BATTERY_ICON_WIDTH 34
#define BATTERY_ICON_HEIGHT 16
#define BATTERY_FILL_REGION_OFFSET_X 5
#define BATTERY_FILL_REGION_OFFSET_Y 5
#define BATTERY_FILL_REGION_OFFSET_WIDTH (BATTERY_ICON_WIDTH-12)
#define BATTERY_FILL_REGION_OFFSET_HEIGHT (BATTERY_ICON_HEIGHT-10)
#define BATTERY_BACK_COLOR 0x00000000
#define BATTERY_FORE_COLOR 0xffffffff

static uint32_t batteryIcon [BATTERY_ICON_HEIGHT][BATTERY_ICON_WIDTH] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};


#define FLASH_ICON_WIDTH 34
#define FLASH_ICON_HEIGHT 16
static uint32_t flashIcon [FLASH_ICON_HEIGHT][FLASH_ICON_WIDTH] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};


#define NOBAT_ICON_WIDTH 34
#define NOBAT_ICON_HEIGHT 16
static uint32_t noBatIcon [NOBAT_ICON_HEIGHT][NOBAT_ICON_WIDTH] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};


/* TODO: this should be in config */
static std::string fileUsbConnected = "/sys/class/power_supply/axp20x-usb/present";
static std::string fileBatConnected = "/sys/class/power_supply/axp20x-battery/present";
static std::string fileBatCapacity = "/sys/class/power_supply/axp20x-battery/capacity";

/*static std::string fileUsbConnected = "/home/vincent/Workspace/axp20x-usb_present";
static std::string fileBatConnected = "/home/vincent/Workspace/axp20x-battery_present";
static std::string fileBatCapacity = "/home/vincent/Workspace/axp20x-battery_capacity";*/

int 	Battery::percentage_ = 0;
int 	Battery::prevPercentage_ = 0;
bool 	Battery::charging_ = false;
bool 	Battery::prevCharging_ = false;
bool 	Battery::noBat_ = false;
bool 	Battery::prevNoBat_ = false;
float 	Battery::currentWaitTime_ = 0.0f;
bool 	Battery::mustRender_ = false;


Battery::Battery(Page &p, float scaleX, float scaleY, float reloadPeriod, SDL_Color fontColor)
    : Component(p)
    , texture_(NULL)
    , texture_prescaled_(NULL)
    , reloadPeriod_(reloadPeriod)
    , scaleX_(scaleX)
    , scaleY_(scaleY)
    , fontColor_(0xff000000 | ((uint32_t)fontColor.b) << 16 | ((uint32_t)fontColor.g) << 8 | ((uint32_t)fontColor.r))
{
    allocateGraphicsMemory();
}

Battery::~Battery()
{
    freeGraphicsMemory();
}

void Battery::freeGraphicsMemory()
{
    Component::freeGraphicsMemory();

    SDL_LockMutex(SDL::getMutex());
    if (texture_ != NULL)
    {
        SDL_FreeSurface(texture_);
        texture_ = NULL;
    }
    if (texture_prescaled_ != NULL)
    {
        SDL_FreeSurface(texture_prescaled_);
	texture_prescaled_ = NULL;
    }
    SDL_UnlockMutex(SDL::getMutex());
}

void Battery::allocateGraphicsMemory()
{

    if(!texture_)
    {
        SDL_LockMutex(SDL::getMutex());

        /* Load image */
        unsigned int rmask;
        unsigned int gmask;
        unsigned int bmask;
        unsigned int amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
#endif
        texture_ = SDL_CreateRGBSurface(0, BATTERY_ICON_WIDTH, BATTERY_ICON_HEIGHT, 32, rmask, gmask, bmask, amask);
        if (!texture_ )
        {
            printf("	Failed-> creating battery icon surface: %s\n", SDL_GetError());
        }


        if (texture_ != NULL)
        {
	    drawBatteryPercent();

	    //SDL_SetAlpha(texture_, SDL_SRCALPHA, 255);

	    /* Set real dimensions */
	    baseViewInfo.ImageWidth = texture_->w * scaleX_;
	    baseViewInfo.ImageHeight = texture_->h * scaleY_;
        }
        SDL_UnlockMutex(SDL::getMutex());

    }

    Component::allocateGraphicsMemory();

}

void Battery::drawBatteryPercent()
{
    int i, j;

    if(texture_ != NULL)
    {
        SDL_LockMutex(SDL::getMutex());

        /* Draw empty battery container */
        uint32_t *texturePixels = (uint32_t*)texture_->pixels;
        for(i = 0; i < BATTERY_ICON_HEIGHT; i++){
	    for(j = 0; j < BATTERY_ICON_WIDTH; j++){
	        uint32_t *currentPixel = texturePixels + BATTERY_ICON_WIDTH*i + j;
		*currentPixel = batteryIcon[i][j] ? fontColor_ : BATTERY_BACK_COLOR;
	    }
        }

        /* Fill battery */
        for(i = 0; i < BATTERY_FILL_REGION_OFFSET_HEIGHT; i++){
	    for(j = 0; j < BATTERY_FILL_REGION_OFFSET_WIDTH; j++){
	        uint32_t *currentPixel = texturePixels +
		  (BATTERY_ICON_WIDTH)*(i+BATTERY_FILL_REGION_OFFSET_Y) +
		  (j+BATTERY_FILL_REGION_OFFSET_X);
		*currentPixel = (j <= BATTERY_FILL_REGION_OFFSET_WIDTH*percentage_/100) ? fontColor_ : BATTERY_BACK_COLOR;
	    }
        }

        /* Force render*/
        mustRender_ = true;

        SDL_UnlockMutex(SDL::getMutex());

    }

    Component::allocateGraphicsMemory();

}

void Battery::drawNoBattery()
{
    int i, j;

    if(texture_ != NULL)
    {
        SDL_LockMutex(SDL::getMutex());

        /* Draw empty battery container */
        uint32_t *texturePixels = (uint32_t*)texture_->pixels;
        for(i = 0; i < NOBAT_ICON_HEIGHT; i++){
	    for(j = 0; j < NOBAT_ICON_WIDTH; j++){
	        uint32_t *currentPixel = texturePixels + BATTERY_ICON_WIDTH*i + j;
		*currentPixel = noBatIcon[i][j] ? fontColor_ : BATTERY_BACK_COLOR;
	    }
        }

        /* Force render*/
        mustRender_ = true;

        SDL_UnlockMutex(SDL::getMutex());

    }

    Component::allocateGraphicsMemory();

}

void Battery::drawBatteryCharging()
{
    int i, j;

    if(texture_ != NULL)
    {
        SDL_LockMutex(SDL::getMutex());

        /* Draw empty battery container */
        uint32_t *texturePixels = (uint32_t*)texture_->pixels;
        for(i = 0; i < FLASH_ICON_HEIGHT; i++){
	    for(j = 0; j < FLASH_ICON_WIDTH; j++){
	        uint32_t *currentPixel = texturePixels + BATTERY_ICON_WIDTH*i + j;
		*currentPixel = flashIcon[i][j] ? fontColor_ : BATTERY_BACK_COLOR;
	    }
        }

        /* Force render*/
        mustRender_ = true;

        SDL_UnlockMutex(SDL::getMutex());

    }

    Component::allocateGraphicsMemory();

}

int Battery::readFileValue(std::string file){

	std::ifstream ifs(file.c_str());
    std::string line;
    int retVal = -1;

	if (!ifs.is_open())
	{
		Logger::write(Logger::ZONE_ERROR, "Configuration", "Could not open " + file + "\"");
		return -1;
	}

	while (std::getline (ifs, line))
	{
		retVal = atoi(line.c_str());
	}

	ifs.close();

	return retVal;
}

bool Battery::isBatConnected(){

	int res = readFileValue(fileBatConnected);
	//printf("%s: %d \n", __func__, res);

	return (res==1)?true:false;
}

bool Battery::isUsbConnected(){

	int res = readFileValue(fileUsbConnected);
	//printf("%s: %d \n", __func__, res);

	return (res==1)?true:false;
}

int Battery::getBatPercent(){

	int res = readFileValue(fileBatCapacity);
	//printf("%s: %d \n", __func__, res);

	return res;
}

void Battery::update(float dt)
{

    if (currentWaitTime_ < reloadPeriod_)
    {
        currentWaitTime_ += dt;
    }
    else if(baseViewInfo.Alpha > 0.0f)
    {
        prevPercentage_ = percentage_;
	prevNoBat_ = noBat_;
	prevCharging_ = charging_;

	percentage_ = getBatPercent();
	noBat_ = !isBatConnected();
	charging_ = isUsbConnected();

	currentWaitTime_ = 0.0f;
    }

    /* Redraw icon if necessary */
	bool stateChange = ((prevNoBat_ != noBat_) || (prevCharging_ != charging_));

    if(noBat_){
        if(prevNoBat_ != noBat_ || stateChange){
	    drawNoBattery();
	}
    }
    else if(charging_){
        if(prevCharging_ != charging_ || stateChange){
	    drawBatteryCharging();
	}
    }
    else if(prevPercentage_ != percentage_ || stateChange){
		float percentagePixelWidth = percentage_ * BATTERY_FILL_REGION_OFFSET_WIDTH / 100;
		float prevPercentagePixelWidth = prevPercentage_ * BATTERY_FILL_REGION_OFFSET_WIDTH / 100;
		if (prevPercentagePixelWidth != percentagePixelWidth || stateChange)
		{
			drawBatteryPercent();
		}
    }

    Component::update(dt);
}


void Battery::draw()
{
	bool scaling_needed = false;
	bool cache_scaling_needed = false;
	bool use_prescaled = false;

    Component::draw();

    if(texture_)
    {
        SDL_Rect rect;
        rect.x = static_cast<int>(baseViewInfo.XRelativeToOrigin());
        rect.y = static_cast<int>(baseViewInfo.YRelativeToOrigin());
        rect.h = static_cast<int>(baseViewInfo.ScaledHeight());
        rect.w = static_cast<int>(baseViewInfo.ScaledWidth());

		/* Cache scaling */
		scaling_needed = rect.w!=0 && rect.h!=0 && (texture_->w != rect.w || texture_->h != rect.h);
		if(scaling_needed){
			cache_scaling_needed = (texture_prescaled_ == NULL)?true:(texture_prescaled_->w != rect.w || texture_prescaled_->h != rect.h);
			if(cache_scaling_needed){
				texture_prescaled_ = SDL::zoomSurface(texture_, NULL, &rect);
				if(texture_prescaled_ == NULL){
					printf("ERROR in %s - Could not create texture_prescaled_\n", __func__);
					use_prescaled = false;
				}
			}

			if(texture_prescaled_ != NULL){
				use_prescaled = true;
			}
		}

		if(use_prescaled && texture_prescaled_ != NULL){
			SDL::renderCopy(texture_prescaled_, baseViewInfo.Alpha, NULL, &rect, baseViewInfo);
		}
		else{
			SDL::renderCopy(texture_, baseViewInfo.Alpha, NULL, &rect, baseViewInfo);
		}
    }
}


bool Battery::mustRender(  )
{
    if ( Component::mustRender(  ) ) return true;

    if ( mustRender_ && baseViewInfo.Alpha > 0.0f )
    {
        mustRender_ = false;
	return true;
    }

    return false;
}
