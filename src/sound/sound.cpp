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
    bool Sound::isLooping(uint32_t sound){
        assert(sounds.contains(sound));
        return ma_sound_is_looping(&sounds[sound]);
    }
    void Sound::setLooping(uint32_t sound, bool isLooping){
        assert(sounds.contains(sound));
        ma_sound_set_looping(&sounds[sound], isLooping);
    }
    void Sound::setVolume(uint32_t sound, float volume){
        assert(sounds.contains(sound));
        ma_sound_set_volume(&sounds[sound], volume);
    }
    Sound::Result Sound::setVolume(float volume){
        engineVolume = volume;
        return convertMaResultToResult(
            ma_engine_set_volume(&soundEngine, volume)
        );
    }
    float Sound::getVolume(uint32_t sound){
        assert(sounds.contains(sound));
        return ma_sound_get_volume(&sounds[sound]);
    }
    float Sound::getVolume(){
        return engineVolume;
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
