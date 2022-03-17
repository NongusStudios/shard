#include <shard/sound/sound.hpp>

#include <shard/utils.hpp>

namespace shard{
    Sound::Sound(){
        shard_abort_ifnot(
            ma_engine_init(NULL, &soundEngine) == MA_SUCCESS
        );
    }
    Sound::~Sound(){
        ma_engine_uninit(&soundEngine);
    }

    uint32_t Sound::load(const std::string& filePath){
        ma_sound& sound = sounds[currentSoundName];
        shard_abort_ifnot(
            ma_sound_init_from_file(
                &soundEngine, filePath.c_str(),
                0, NULL, NULL,
                &sound
            ) == MA_SUCCESS
        );
        return currentSoundName++;
    }

    bool Sound::isPlaying(uint32_t sound){
        assert(sounds.contains(sound));
        return ma_sound_is_playing(&sounds[sound]);
    }
    Sound::Result Sound::setVolume(float volume){
        return convertMaResultToResult(
            ma_engine_set_volume(&soundEngine, volume)
        );
    }
    Sound::Result Sound::play(const std::string& filePath){
        return convertMaResultToResult(
            ma_engine_play_sound(&soundEngine, filePath.c_str(), NULL)
        );
    }
    Sound::Result Sound::play(uint32_t sound){
        assert(sounds.contains(sound));
        auto& s = sounds[sound];
        if(!ma_sound_is_playing(&s)){
            return convertMaResultToResult(
                ma_sound_start(&s)
            );
        }
        return Result::FAILED;
    }

    Sound::Result Sound::start(){
        return convertMaResultToResult(
            ma_engine_start(&soundEngine)
        );
    }
    Sound::Result Sound::stop(){
        return convertMaResultToResult(
            ma_engine_stop(&soundEngine)
        );
    }
} // namespace shard
