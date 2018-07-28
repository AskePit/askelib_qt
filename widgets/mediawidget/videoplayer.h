#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QTimer>
#include <vlc/libvlc.h>
#include <vlc_common.h>
#include <vlc/libvlc_media.h>

struct libvlc_instance_t;
struct libvlc_media_player_t;
struct libvlc_event_manager_t;
struct libvlc_event_t;
struct vlc_object_t;

class QSlider;
class QLabel;

namespace aske
{

/*!
 * @brief The VideoView widget.
 * @details
 * Used as a viewport for video-stream.
 * Managed by `VideoPlayer`
 *
 * @see VideoPlayer
 */
class VideoView : public QWidget
{
    Q_OBJECT
public:
    explicit VideoView(QWidget *parent = 0);

    /*! Returns window identificator. */
    WId window();
};

/*!
 * @brief Video Player.
 * @details
 * Uses VideoView as a viewport for video-stream, manages stream.
 * Optionally can use QSlider for video-progress, QSlider for volume and
 * QLabel for errors. All these widget should be provided from outside by
 * `setWidgets` function.
 *
 * @see VideoView
 */
class VideoPlayer : public QObject
{
    Q_OBJECT

public:
    enum Direction
    {
        Backward = 0,
        Forward,
    };

    VideoPlayer();
    ~VideoPlayer();

    /*!
     * Sets all necessary widgets for player's functionality.
     * `progress`, `volume` and `codecErrorLabel` are optional.
     */
    void setWidgets(VideoView *view, QSlider *progress, QSlider *volume, QLabel *codecErrorLabel);

    /*! Loads video-file and automatically plays it. */
    bool load(const QString &file);

    /*! Play video. */
    void play();

    /*! Play video from start. */
    void replay();

    /*! Rewind video bacwards/forwards with specified `step`. */
    void rewind(Direction dir, qreal step);

    /*! Pause video */
    void pause();

    /*! Resume video. If video has reached end, restart it. */
    void resume();

    /*! Pause/resume video. */
    void toggle();

    /*! Stop playing video. */
    void stop();

    /*! Current video-stream state. */
    libvlc_state_t state() const;

    /*! Current audio-volume. */
    int volume() const;

    /*! Set current audio-volume. */
    void setVolume(int volume);

    /*! Current video-position. */
    qreal position() const;

    /*! Set current video-position. */
    void setPosition(qreal pos);

    /*! Video dimensions. */
    const QSize size() const;

    /*! Triggers position/volume sliders show for a some period of time. */
    void showSliders();

signals:
    /*! Video is loaded. */
    void loaded();

    /*! Audio-volume has been changed to `value`. */
    void volumeChanged(qreal value);

    /*! Audio has been muted/unmuted. */
    void muteChanged(bool mute);

private:
    libvlc_instance_t *m_vlc {nullptr};
    libvlc_media_player_t *m_vlcMediaPlayer {nullptr};
    libvlc_media_t *m_vlcMedia {nullptr};
    libvlc_event_manager_t *m_vlcEvents {nullptr};

    static void libvlc_callback(const libvlc_event_t *event, void *data);

    VideoView *m_view;

    QSlider *m_progressSlider {nullptr};
    QSlider *m_volumeSlider {nullptr};
    QLabel *m_codecErrorLabel {nullptr};

    QString m_currentFile;
    QTimer m_slidersTimer;
    bool m_userChangedVideoPos {false};
    bool m_firstLoad {true};

    static int volumeCallback(vlc_object_t *, const char *, vlc_value_t, vlc_value_t, void *);
    static int muteCallback(vlc_object_t *, const char *, vlc_value_t, vlc_value_t, void *);
};

} // namespace aske

#endif // VIDEOPLAYER_H
