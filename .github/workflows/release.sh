#!/usr/bin/env bash
set -ex
shopt -s extglob
cd "$(dirname -- "$(readlink -f -- "$0")")"
cd ../..

if [[ $GITHUB_REF == refs/heads/* ]]
then
    BRANCH_NAME="${GITHUB_REF#refs/heads/}"
    TAG_NAME=""
elif [[ $GITHUB_REF == refs/tags/* ]]
then
    BRANCH_NAME=""
    TAG_NAME="${GITHUB_REF#refs/tags/}"
fi

if [[ -z $BRANCH_NAME && -z $TAG_NAME ]]
then
    echo "Unknown GITHUB_REF: $GITHUB_REF"
    exit 1
fi

EDGE_TAG="edge-$BRANCH_NAME"
EDGE_RELEASE_TITLE="[$BRANCH_NAME] Continuous build"
EDGE_RELEASE_NOTES='This is an automated build from the latest changes in the `'"$BRANCH_NAME"'` branch. Stability is not guaranteed. Use at your own risk.'
FILES=(artifacts/*/*)
GH_VER=1.1.0

curl -SsL "https://github.com/cli/cli/releases/download/v$GH_VER/gh_${GH_VER}_linux_amd64.tar.gz" \
    | tar -xzOf- "gh_${GH_VER}_linux_amd64/bin/gh" \
    | sudo tee "$(which gh)" > /dev/null

function getTagId
{
    TAG_NAME="$1"

    TAG_ID="$(
        gh api graphql -F owner=':owner' -F name=':repo' -F tagName="$TAG_NAME" -f query='
            query($name: String!, $owner: String!, $tagName: String!) {
                repository(owner: $owner, name: $name) {
                    ref (qualifiedName: $tagName) {
                        id
                    }
                }
            }
        ' | jq -r '.data.repository.ref.id'
    )"

    if [[ $TAG_ID == null ]]
    then
        echo "tag \"$TAG_NAME\" not found" >&2
        echo ""
    else
        echo -n "$TAG_ID"
    fi
}

function removeTagById
{
    TAG_ID="$1"

    RES="$(
        gh api graphql -F tagId="$TAG_ID" -F mutationId="abc" -f query='
            mutation($tagId: ID!, $mutationId: String!){
                deleteRef(input:{refId: $tagId, clientMutationId: $mutationId}) {
                    clientMutationId
                }
            }
        ' | jq -r '.data.deleteRef.clientMutationId'
    )"

    if [[ $RES == null ]]
    then
        echo "cannot remove tag id \"$TAG_ID\"" >&2
        exit 1
    fi
}


if [[ -z $TAG_NAME ]]
then
    echo "No current tag. Assuming edge release."
    TAG_ID="$(getTagId "$EDGE_TAG")"
    if [[ $TAG_ID ]]
    then
        gh release delete edge -y || true
        removeTagById "$TAG_ID"
    fi
    GH_FILES=()
    for FILE in "${FILES[@]}"
    do
        FILENAME="$(basename -- "$FILE")"
        FILE_TITLE="${FILENAME//-v+([[:digit:]])*([^-])-/-$EDGE_TAG-}"
        GH_FILES+=("$FILE#$FILE_TITLE")
    done
    gh release create "$EDGE_TAG" "${GH_FILES[@]}" -p -t "$EDGE_RELEASE_TITLE" -n "$EDGE_RELEASE_NOTES"
else
    echo "Releasing tag: $TAG_NAME"
    TAG_ID="$(getTagId "$TAG_NAME")"
    if [[ -z $TAG_ID ]]
    then
        echo "Tag not found: $TAG_NAME"
        exit 1
    fi
    RELEASE_DESC="$(grep -Pzom1 "(?s)\n[#\s]*$TAG_NAME.*?\n+.*?\K.*?\n\n\n" CHANGELOG.md)"
    gh release create "$TAG_NAME" "${FILES[@]}" CHANGELOG.md README.md -t "$TAG_NAME" -n "$RELEASE_DESC"
fi
