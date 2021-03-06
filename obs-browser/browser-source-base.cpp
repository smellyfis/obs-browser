/******************************************************************************
 Copyright (C) 2014 by John R. Bradley <jrb@turrettech.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include <memory>
#include <map>

#include "browser-source.hpp"
#include "browser-listener.hpp"
#include "browser-manager.hpp"

#include "browser-source-base.hpp"
#include "browser-source-listener-base.hpp"

BrowserSource::BrowserSource(obs_data_t *settings, obs_source_t *source)
: pimpl(new Impl(this)), source(source), browserIdentifier(0)
{
	pthread_mutex_init(&textureLock, NULL);
	UpdateSettings(settings);
}


BrowserSource::~BrowserSource()
{
	if (browserIdentifier != 0) {
		BrowserManager::Instance()->DestroyBrowser(browserIdentifier);
	}
	pthread_mutex_destroy(&textureLock);
}

std::shared_ptr<BrowserListener> BrowserSource::CreateListener()
{
	return pimpl->CreateListener();
}

void BrowserSource::RenderActiveTexture(gs_effect_t *effect)
{
	pimpl->RenderCurrentTexture(effect);
}

void BrowserSource::InvalidateActiveTexture()
{
	pimpl->InvalidateActiveTexture();
}

BrowserSource::Impl::Impl(BrowserSource *parent)
: parent(parent), activeTexture(nullptr)
{}

BrowserSource::Impl::~Impl()
{}

void BrowserSource::Impl::RenderCurrentTexture(gs_effect_t *effect)
{
	GetParent()->LockTexture();

	if (activeTexture != nullptr) {
		gs_reset_blend_state();
		gs_effect_set_texture(
				gs_effect_get_param_by_name(effect, "image"),
				activeTexture);
		gs_draw_sprite(activeTexture, 0, parent->GetWidth(), 
				parent->GetHeight());
	}
	
	GetParent()->UnlockTexture();
}

void BrowserSource::Impl::InvalidateActiveTexture()
{
	activeTexture = nullptr;
}

std::shared_ptr<BrowserListener> BrowserSource::Impl::CreateListener()
{
	return std::shared_ptr<BrowserListener>(
			new BrowserSource::Impl::Listener(this));
}
