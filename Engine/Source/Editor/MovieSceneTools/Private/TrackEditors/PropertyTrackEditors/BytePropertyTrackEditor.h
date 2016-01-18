// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MovieSceneByteTrack.h"
#include "MovieSceneByteSection.h"


/**
* A property track editor for byte and enums.
*/
class FBytePropertyTrackEditor
	: public FPropertyTrackEditor<UMovieSceneByteTrack, UMovieSceneByteSection, uint8>
{
public:

	/**
	 * Constructor.
	 *
	 * @param InSequencer The sequencer instance to be used by this tool
	 */
	FBytePropertyTrackEditor( TSharedRef<ISequencer> InSequencer)
		: FPropertyTrackEditor( InSequencer, NAME_ByteProperty )
	{ }

	/**
	 * Creates an instance of this class.  Called by a sequencer 
	 *
	 * @param OwningSequencer The sequencer instance to be used by this tool
	 * @return The new instance of this class
	 */
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor( TSharedRef<ISequencer> OwningSequencer );

public:

	// ISequencerTrackEditor interface

	virtual UMovieSceneTrack* AddTrack( UMovieScene* FocusedMovieScene, const FGuid& ObjectHandle, TSubclassOf<class UMovieSceneTrack> TrackClass, FName UniqueTypeName ) override;

protected:

	// FPropertyTrackEditor interface

	virtual TSharedRef<FPropertySection> MakePropertySectionInterface( UMovieSceneSection& SectionObject, UMovieSceneTrack& Track ) override;
	virtual void GenerateKeysFromPropertyChanged( const FPropertyChangedParams& PropertyChangedParams, TArray<uint8>& GeneratedKeys ) override;
};
