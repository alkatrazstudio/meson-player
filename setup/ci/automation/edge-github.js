import {recreatePreReleaseWithTag} from './lib/github.js'
import {getRoot, filesByGlob} from './lib/util.js'

const RELEASE_TAG = 'edge-release'
const rootDir = getRoot('../../..')

;(async () => {
    const body = `This is an automated build from the latest changes in the DEV branch. Use only for testing.`
    const filenames = await filesByGlob([
        `${rootDir}/release-linux/*.AppImage`
    ])
    await recreatePreReleaseWithTag(RELEASE_TAG, body, filenames)
})()