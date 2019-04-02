#include "internalgamecontroller.h"
#include "protobuf/ssl_referee.h"
#include <google/protobuf/descriptor.h>
#include <QDebug>

InternalGameController::InternalGameController(const Timer *timer) :
    m_timer(timer)
{
    m_trigger = new QTimer(this);
    connect(m_trigger, &QTimer::timeout, this, &InternalGameController::sendUpdate);
    m_trigger->start(UPDATE_INTERVAL_MS);

    connect(timer, &Timer::scalingChanged, this, &InternalGameController::setScaling);

    m_packet.set_stage(SSL_Referee::NORMAL_FIRST_HALF);
    m_packet.set_command(SSL_Referee::HALT);
    m_packet.set_command_counter(0);
    m_packet.set_command_timestamp(timer->currentTime());
    teamInfoSetDefault(m_packet.mutable_blue());
    teamInfoSetDefault(m_packet.mutable_yellow());
}

void InternalGameController::sendUpdate()
{
    // update packet
    m_packet.set_packet_timestamp(m_timer->currentTime());
    // TODO: is stage_time_left needed?

    // send packet to internal referee
    QByteArray packetData;
    packetData.resize(m_packet.ByteSize());
    if (m_packet.SerializeToArray(packetData.data(), packetData.size())) {
        emit gotPacketForReferee(packetData);
    }
}

void InternalGameController::setScaling(double scaling)
{
    // update scaling as told
    if (scaling <= 0) {
        m_trigger->stop();
    } else {
        const int t = UPDATE_INTERVAL_MS / scaling;
        m_trigger->start(qMax(1, t));
    }
}

void InternalGameController::handleGuiCommand(const QByteArray &data)
{
    SSL_Referee newState;
    newState.ParseFromArray(data.data(), data.size());

    // the ui part of the internal referee will only change command, stage or goalie
    if (newState.command() != m_packet.command() || newState.stage() != m_packet.stage()) {
        // clear all internal state
        auto counterBefore = m_packet.command_counter();
        m_packet.CopyFrom(newState);
        m_packet.set_command_timestamp(m_timer->currentTime());
        m_packet.set_command_counter(counterBefore+1);
    } else {
        m_packet.mutable_blue()->CopyFrom(newState.blue());
        m_packet.mutable_yellow()->CopyFrom(newState.yellow());
    }

    sendUpdate();
}

void InternalGameController::handleStatus(const Status& status)
{
    if (status->has_geometry()) {
        m_geometry = status->geometry();
    }
}

void InternalGameController::handleCommand(const amun::CommandReferee &refereeCommand)
{
    if (refereeCommand.has_command()) {
        const std::string &c = refereeCommand.command();
        handleGuiCommand(QByteArray(c.data(), c.size()));
    }
}

auto InternalGameController::ballPlacementPosForFoul(Vector foulPosition) -> Vector
{

}

void InternalGameController::handleGameEvent(std::shared_ptr<gameController::AutoRefToController> message)
{
    if (!message->has_game_event()) {
        // TODO: emit reply message
        return;
    }
    const gameController::GameEvent &event = message->game_event();

    // extract location and team name
    std::string byTeamString;
    Vector eventLocation{0, 0};

    const google::protobuf::Reflection *refl = event.GetReflection();
    const google::protobuf::Descriptor *desc = gameController::GameEvent::descriptor();
    // extract fields using reflection
    for (int i = 0; i < desc->field_count(); i++) {
        const google::protobuf::FieldDescriptor *field = desc->field(i);
        if (field->name() == "type" || field->name() == "origin") {
            // ignore them as they are not events
            continue;
        }
        if (refl->HasField(event, field)) {
            const google::protobuf::Message &eventMessage = refl->GetMessage(event, field);
            const google::protobuf::Reflection *messageRefl = eventMessage.GetReflection();
            const google::protobuf::Descriptor *messageDesc = eventMessage.GetDescriptor();

            for (int b = 0;b < messageDesc->field_count(); b++) {
                const google::protobuf::FieldDescriptor *field = messageDesc->field(b);
                std::string fieldName = field->name();
                if (fieldName == "by_team") {
                    byTeamString = messageRefl->GetEnum(eventMessage, field)->name();
                } else if (fieldName == "location") {
                    const google::protobuf::Message &locationMessage = messageRefl->GetMessage(eventMessage, field);
                    const gameController::Location &location = static_cast<const gameController::Location&>(locationMessage);
                    eventLocation.x = location.x();
                    eventLocation.y = location.y();
                }
            }
        }
    }
}
